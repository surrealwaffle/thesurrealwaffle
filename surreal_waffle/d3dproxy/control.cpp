#define WIN32_MEAN_AND_LEAN
#include <Windows.h>

#include "control.h"
#include <math.h>

// Pointer to key state table, indexed by DIK_*.
// A key takes on a non-zero value when that key is down.
unsigned __int8 *const	g_lpKeys = (unsigned __int8*)(0x0064C570);

// A table of values where each key, indexed by DIK_*, is:
//	0: no event occurred since last poll
//	1: key was just pressed
//	-1: key was just released
__int8					iKeyEvent[0xDD] = { 0 };

// Pointer to the local player's position in the world, indexed by (0,1,2)->(x,y,z).
float3&					g_lpWorld = *((float3*)(0x402AB0C4));

// Pointer to the local player's orientation, by angles, indexed by (0,1)->(yaw, pitch).
float2&					g_lpOrientation = *((float2*)(0x402AD4B8));

float3 cross( float2 const &u, float2 const &v ) {
	return float3(
		u[1]*v[2] - u[2]*v[1],
		u[2]*v[0] - u[0]*v[2],
		u[0]*v[1] - u[1]*v[0]
	);
}

bool RunTo( float3 const &P3, float const tol ) {
	// project p3 onto p2
	// we can't really do anything about Z other than jump anyway (a later consideration)
	float2 const P( P3 );

	// player position
	float2 const P0( ::g_lpWorld );

	// (ignore the z coord)
	if ( norm2( P - P0 ) <= tol )
		return true;

	// the unit vector for the direction to P from the player's position
	float2 const 
		facing = float2( cos( ::g_lpOrientation[ 0 ] ), ::sin( g_lpOrientation[ 0 ] ) ), // forwarding facing direction
		direct = normalize( P - P0 ); // the unit vector for the direction to P from the player's position
	float const 
		forwardBias = dot( direct, facing ),
		leftBias = (cross( facing, direct ))[2];

	// use my interject library later to make this easier to do without resetting values
	if ( forwardBias > 0.2f ) {
		GetKey( KEY_W ) = 2;
		GetKey( KEY_S ) = 0;
	} else if ( forwardBias < -0.05 ) {
		GetKey( KEY_W ) = 0;
		GetKey( KEY_S ) = 2;
	}

	
	if ( leftBias > 0.2f ) {
		GetKey( KEY_A ) = 2;
		GetKey( KEY_D ) = 0;
	} else if ( leftBias < -0.05 ) {
		GetKey( KEY_A ) = 0;
		GetKey( KEY_D ) = 2;
	}
	

	return false;
}

// Updates a key event table for GetKeyEvent( ).
void PollKeys( void ) {
	for ( size_t i = 0; i < ( sizeof( ::iKeyEvent ) / sizeof( ::iKeyEvent[ 0 ] ) ); i++ ) {
		unsigned __int8 const keyState = GetKey( i );
		__int8 &keyEvent = ::iKeyEvent[ i ];
		switch ( keyState ) {
		case 0:	// key has been or is unpressed
			keyEvent = keyEvent == 0 ? KEYEVENT_NONE : KEYEVENT_RELEASED;
			break;
		case 1: // key just pressed
			keyEvent = KEYEVENT_PRESSED;
			break;
		default: // key is held
			keyEvent = KEYEVENT_NONE;
			break;
		}
	}
}

// Returns the event of key i.
// The events are specified by KEYEVENT_*.
__int8 GetKeyEvent( size_t const i ) {
	return ::iKeyEvent[ i ];
}

// Returns a reference to key i in g_lpKey.
unsigned __int8& GetKey( size_t const i ) {
	return ::g_lpKeys[ i ];
}

void __declspec( naked ) SubmitChatDelegate( wchar_t const *const lpwszText ) {
	typedef void(*CALL_ADDRESS)(void);
	static CALL_ADDRESS CALL_1 = (CALL_ADDRESS)(0x005CAE9A);
    static CALL_ADDRESS CALL_3 = (CALL_ADDRESS)(0x005CCE23);
    static CALL_ADDRESS CALL_2 = (CALL_ADDRESS)(0x004ADF70);

	__asm {
		// ensure not in main menu
		MOV		AL, BYTE PTR DS:[0x402AAFC0]
		TEST	AL, AL
		JZ		_BEGIN
		RETN

_BEGIN:
        // make ESI point to text
        MOV ESI,DWORD PTR SS:[ESP+4]

        PUSH ESI
        CALL CALL_1
        ADD ESP,4
        CMP EAX,0x0FE
        JBE _REP1
        MOV EDI,0x0FE
        JMP _REP2
_REP1:
        PUSH ESI
        CALL CALL_1
        ADD ESP,4
        MOV EDI,EAX
_REP2:
        PUSH EDI //string length
        LEA EDX,DWORD PTR SS:[ESP+0x10] //(replaced below)
        PUSH ESI
        PUSH EDX
        CALL CALL_3
        //MOV EAX,DWORD PTR DS:[0x0064E7AC] // FFFFFFFF if no chat bar
        MOV EAX, 0x00000000 // pretend that the chat control is open
        PUSH EBX
        LEA EDX,DWORD PTR SS:[ESP+0x1C]
        MOV WORD PTR SS:[ESP+EDI*2+0x1C],0
        CALL CALL_2
        ADD ESP,0x10
        RETN
	}
}

void __cdecl SubmitChat( wchar_t const *const lpwszText ) {
	static DWORD lastSubmit = 0;
	// skip condition to avoid crash

	DWORD const t = GetTickCount( );
	if ( t - lastSubmit > 50 ) {
		lastSubmit = t;

		__asm {
			PUSHAD
			PUSHFD
		
			SUB ESP, 0x200
			PUSH lpwszText
			CALL SubmitChatDelegate
			ADD ESP, 0x204


			POPFD
			POPAD
		}
	}
}

float dot( float3 const &a, float3 const &b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float norm2( float3 const &a ) {
	return dot( a, a );
}

float norm( float3 const &a ) {
	return sqrt( norm2( a ) );
}

float3 normalize( float3 const &a ) {
	return (1 / norm( a ) ) * a;
}

float3 operator*( float const s, float3 const &v) {
	return float3( s * v[0], s * v[1], s * v[2] );
}


float3 operator+( float3 const &a, float3 const &b ) {
	return float3( a[0] + b[0], a[1] + b[1], a[2] + b[2] );
}

float3 operator-( float3 const &a, float3 const &b ) {
	return float3( a[0] - b[0], a[1] - b[1], a[2] - b[2] );
}

float dot( float2 const &a, float2 const &b) {
	return a[0] * b[0] + a[1] * b[1];
}

float norm2( float2 const &a ) {
	return dot( a, a );
}

float norm( float2 const &a ) {
	return sqrt( norm2( a ) );
}

float2 normalize( float2 const &a ) {
	return (1 / norm( a ) ) * a;
}

float2 operator*( float const s, float2 const &v) {
	return float2( s * v[0], s * v[1] );
}


float2 operator+( float2 const &a, float2 const &b ) {
	return float2( a[0] + b[0], a[1] + b[1] );
}

float2 operator-( float2 const &a, float2 const &b ) {
	return float2( a[0] - b[0], a[1] - b[1] );
}
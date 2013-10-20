#ifndef CONTROL_H
#define CONTROL_H

/*
	control.h:
		Provides a very rough interface to Halo.
*/

#define KEYEVENT_NONE		0
#define KEYEVENT_PRESSED	1
#define KEYEVENT_RELEASED	2

/******************
 * Classes and Structures (Simple)
 ******************/

// honestly should just template...
// unfortunately no variadic templates, so its kind of a waste still

class float3 {
private:
	float f[3];
public:
	float3( float x, float y, float z ) {
		f[0] = x;
		f[1] = y;
		f[2] = z;
	}

	float& operator[]( size_t const i ) { return f[ i ]; }
	float const& operator[]( size_t const i ) const { return f[ i ]; }
};

class float2 {
private:
	float f[2];
public:
	float2( float3 const &v ) {
		f[ 0 ] = v[ 0 ];
		f[ 1 ] = v[ 1 ];
	}

	float2( float x, float y ) {
		f[0] = x;
		f[1] = y;
	}

	float& operator[]( size_t const i ) { return f[ i ]; }
	float const& operator[]( size_t const i ) const { return f[ i ]; }
};


/******************
 * Globals
 ******************/

#define KEY_ESC	0x00
#define KEY_F1	0x01
#define KEY_F2	0x02
#define KEY_F3	0x03
#define KEY_F4	0x04
#define KEY_F5	0x05
#define KEY_F6	0x06
#define KEY_F7	0x07
#define KEY_F8	0x08
#define KEY_F9	0x09
#define KEY_F10	0x0A
#define KEY_F11	0x0B
#define KEY_F12	0x0C

#define KEY_W	0x20
#define KEY_A	0x2D
#define KEY_S	0x2E
#define KEY_D	0x2F


// Pointer to key state table, indexed by KEY_*.
// A key takes on a non-zero value when that key is down.
extern unsigned __int8 *const	g_lpKeys;

// Pointer to the local player's position in the world, indexed by (0,1,2)->(x,y,z).
extern float3&					g_lpWorld;

// Pointer to the local player's orientation, by angles, indexed by (0,1)->(yaw, pitch).
extern float2&					g_lpOrientation;



/******************
 * Prototypes
 ******************/

float dot( float3 const&, float3 const& );
float norm2( float3 const& );
float norm( float3 const& );
float3 normalize( float3 const& );
float3 operator*( float const, float3 const& );
float3 operator+( float3 const&, float3 const& );
float3 operator-( float3 const&, float3 const& );

float dot( float2 const&, float2 const& );
float norm2( float2 const& );
float norm( float2 const& );
float2 normalize( float2 const& );
float2 operator*( float const, float2 const& );
float2 operator+( float2 const&, float2 const& );
float2 operator-( float2 const&, float2 const& );

float3 cross( float3 const&, float2 const& );

// Moves the player in the direction of the supplied point.
// Returns true iff the player is within tol^2 game units of the supplied points.
// In this case, the player is not moved by this function.
bool RunTo( float3 const&, float const tol = 0.2f * 0.2f );

// Updates a key event table for GetKeyEvent( ).
void PollKeys( void );

// Returns the event of key i.
// The events are specified by KEYEVENT_*.
__int8 GetKeyEvent( size_t const i );

// Returns a reference to key i in g_lpKey.
unsigned __int8& GetKey( size_t const i );

// Enters the text into all chat and submits it.
void __cdecl SubmitChat( wchar_t const *const lpwszText );

#endif // KEYS_H
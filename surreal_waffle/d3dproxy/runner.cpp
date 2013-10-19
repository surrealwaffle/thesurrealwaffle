#include "runner.h"

bool bIsRunning = false;
float3 pObjective = float3( 0.f, 0.f, 0.f );

// Sets the runner's objective.
void SetObjective( float3 const &P ) {
	pObjective = P;
	bIsRunning = true;
}

// Causes the runner to move towards the objective.
// Returns values from RUNUPDATE_*.
int UpdateRunner( void ) {
	if ( bIsRunning ) {
		bool const complete = RunTo( pObjective );

		int retcode = complete ? RUNUPDATE_REACHED : RUNUPDATE_BUSY;

		if ( retcode == RUNUPDATE_REACHED ) {
			GetKey( KEY_W ) = 0;
			GetKey( KEY_A ) = 0;
			GetKey( KEY_S ) = 0;
			GetKey( KEY_D ) = 0;

			bIsRunning = false;
		}

		return retcode;
	}

	return RUNUPDATE_IDLE;
}

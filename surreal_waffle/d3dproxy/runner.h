#ifndef RUNNER_H
#define RUNNER_H

#include "control.h"

// Sets the runner's objective.
void SetObjective( float3 const &P );

#define RUNUPDATE_IDLE		0
#define RUNUPDATE_REACHED	1
#define RUNUPDATE_BUSY		2

// Causes the runner to move towards the objective.
// Returns values from RUNUPDATE_*.
int UpdateRunner( void );

#endif // RUNNER_H
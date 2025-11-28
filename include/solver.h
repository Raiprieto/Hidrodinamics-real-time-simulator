#ifndef SOLVER_H
#define SOLVER_H

#include "state.h"

void Solver_Init(SimulationState *state);
void Solver_Step(SimulationState *state);
void Solver_Cleanup(SimulationState *state);

#endif
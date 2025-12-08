#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "state.h"

// Inicializa el archivo CSV con los encabezados
void Analysis_Init(void);

// Calcula y guarda métricas globales (Energía, Masa)
// Retorna el 'residual' (cambio promedio de velocidad) para ver convergencia
float Analysis_ComputeAndSave(SimulationState *state, int time_step);

// Guarda el estado completo de la grilla (para abrir con Python/Matlab/Paraview)
void Analysis_SaveSnapshot(SimulationState *state, int time_step);

// Inicializa el log de performance
void Analysis_InitPerformanceLog(void);

// Guarda el tiempo de procesamiento de los ultimos N pasos
void Analysis_LogPerformance(int step, double duration);

#endif
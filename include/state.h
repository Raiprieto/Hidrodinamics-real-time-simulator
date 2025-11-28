#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

// Resolución de la simulación (bajamos un poco para que vaya rápido en CPU)
#define GRID_W 200
#define GRID_H 100

// Modelo D2Q9 (9 velocidades)
#define Q 9

typedef struct {
    // Arrays planos para performance
    float *f;       // Distribución actual
    float *new_f;   // Distribución siguiente
    
    float *rho;     // Densidad
    float *ux;      // Velocidad X
    float *uy;      // Velocidad Y
    
    bool *barrier;  // Obstáculos (paredes)
} SimulationState;

// Helper para obtener índice 1D
static inline int idx(int x, int y) {
    if (x < 0) x = 0; if (x >= GRID_W) x = GRID_W - 1;
    if (y < 0) y = 0; if (y >= GRID_H) y = GRID_H - 1;
    return y * GRID_W + x;
}

#endif
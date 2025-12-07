#include "solver.h"
#include <stdlib.h>
#include <math.h>

// Constantes LBM D2Q9
const float w[9] = {4.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/9, 1.0/36, 1.0/36, 1.0/36, 1.0/36};
const int cx[9] = {0, 1, 0, -1, 0, 1, -1, -1, 1};
const int cy[9] = {0, 0, 1, 0, -1, 1, 1, -1, -1};
const int opp[9] = {0, 3, 4, 1, 2, 7, 8, 5, 6}; // Dirección opuesta para rebote

void Solver_Init(SimulationState *state) {
    int N = GRID_W * GRID_H;
    state->f = (float*)calloc(N * Q, sizeof(float));
    state->new_f = (float*)calloc(N * Q, sizeof(float));
    state->rho = (float*)calloc(N, sizeof(float));
    state->ux = (float*)calloc(N, sizeof(float));
    state->uy = (float*)calloc(N, sizeof(float));
    state->barrier = (bool*)calloc(N, sizeof(bool));

    // Inicializar fluido quieto con densidad 1.0
    for (int i = 0; i < N; i++) {
        state->rho[i] = 1.0f;
        for (int k = 0; k < Q; k++) {
            state->f[i*Q + k] = w[k];
            state->new_f[i*Q + k] = w[k];
        }
    }
}

void Solver_Step(SimulationState *state) {
    int N = GRID_W * GRID_H;

    // 1. STREAMING (Propagación) + COLISIÓN BGK
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int i = idx(x, y);

            // Si es barrera, no calculamos física compleja
            if (state->barrier[i]) {
                // Bounce-back simple (reflejo) se maneja al leer los vecinos
                continue;
            }

            // --- MACROSCOPIC (Calcular u y rho) ---
            float rho = 0.0f;
            float ux = 0.0f;
            float uy = 0.0f;

            // Leer distribuciones de los vecinos (STREAMING implícito)
            for (int k = 0; k < Q; k++) {
                // Celda vecina desde donde viene la partícula
                int nx = x - cx[k];
                int ny = y - cy[k];
                
                float f_val;
                if (nx >= 0 && nx < GRID_W && ny >= 0 && ny < GRID_H) {
                    int neighbor_idx = idx(nx, ny);
                    
                    if (state->barrier[neighbor_idx]) {
                        // Si el vecino es pared, rebota la partícula que iba hacia allá
                        // Leemos de NOSOTROS mismos en la dirección opuesta (opp[k])
                        f_val = state->f[i*Q + opp[k]]; 
                    } else {
                        f_val = state->f[neighbor_idx*Q + k];
                    }
                } else {
                    f_val = w[k]; // Fronteras abiertas simples
                }

                // Sumamos para momentos macroscópicos
                rho += f_val;
                ux  += f_val * cx[k];
                uy  += f_val * cy[k];
                
                // Guardamos temporalmente en new_f para usarlo en collision
                state->new_f[i*Q + k] = f_val;
            }
            
            // Normalizar velocidad
            if (rho > 0) {
                ux /= rho;
                uy /= rho;
            }

            // --- INLET (Viento desde la izquierda) ---
            if (x == 0) {
                ux = 0.1f; // Velocidad de entrada
                uy = 0.0f;
                rho = 1.0f;
            }

            state->rho[i] = rho;
            state->ux[i] = ux;
            state->uy[i] = uy;

            // --- COLLISION (Relajación al equilibrio) ---
            for (int k = 0; k < Q; k++) {
                float cu = 3.0f * (cx[k]*ux + cy[k]*uy);
                float f_eq = w[k] * rho * (1.0f + cu + 0.5f*(cu*cu) - 1.5f*(ux*ux + uy*uy));
                
                // Relajación (omega = 1.9 para baja viscosidad)
                float omega = 1.8f; 
                state->new_f[i*Q + k] = (1.0f - omega) * state->new_f[i*Q + k] + omega * f_eq;
            }
        }
    }

    // 2. Actualizar punteros (Swap)
    float *temp = state->f;
    state->f = state->new_f;
    state->new_f = temp;
}

void Solver_Cleanup(SimulationState *state) {
    free(state->f); free(state->new_f);
    free(state->rho); free(state->ux); free(state->uy);
    free(state->barrier);
}
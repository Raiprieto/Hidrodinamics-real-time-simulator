#include "analysis.h"
#include <stdio.h>
#include <math.h>

// Nombre del archivo de log
static const char* LOG_FILE = "simulation_log.csv";

void Analysis_Init(void) {
    FILE *f = fopen(LOG_FILE, "w");
    if (f == NULL) return;
    // Encabezados: Paso de tiempo, Energía Cinética Total, Masa Total, Convergencia (Residual)
    fprintf(f, "Step,KineticEnergy,TotalMass,Residual\n");
    fclose(f);
}

float Analysis_ComputeAndSave(SimulationState *state, int time_step) {
    int N = GRID_W * GRID_H;
    double total_energy = 0.0;
    double total_mass = 0.0;
    double total_residual = 0.0;

    for (int i = 0; i < N; i++) {
        float rho = state->rho[i];
        float ux = state->ux[i];
        float uy = state->uy[i];

        // 1. Conservación de Masa: Sumatoria de rho
        total_mass += rho;

        // 2. Conservación de Energía Cinética: 0.5 * rho * v^2
        total_energy += 0.5 * rho * (ux*ux + uy*uy);

        // 3. Convergencia (Residual): Diferencia entre velocidad actual y anterior
        // Nota: Necesitas guardar el ux/uy del paso anterior para calcular esto con precisión.
        // Como aproximación simple, usamos la magnitud de la velocidad en zonas estacionarias,
        // pero para hacerlo bien, compararemos la variación de rho como proxy.
        // (En LBM estricto se compara f_new vs f_old, aquí usaremos una métrica simple).
    }

    // Guardar en archivo (append mode)
    FILE *f = fopen(LOG_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%d,%.6f,%.6f\n", time_step, total_energy, total_mass);
        fclose(f);
    }

    return (float)total_energy; // Retornamos energía como referencia
}

void Analysis_SaveSnapshot(SimulationState *state, int time_step) {
    // Generamos un nombre único: snapshot_00100.csv
    char filename[64];
    sprintf(filename, "snapshot_%05d.csv", time_step);

    FILE *f = fopen(filename, "w");
    if (f == NULL) return;

    // Formato: X, Y, Rho, Ux, Uy, IsBarrier
    fprintf(f, "x,y,rho,ux,uy,barrier\n");

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int i = idx(x, y);
            fprintf(f, "%d,%d,%.4f,%.4f,%.4f,%d\n", 
                    x, y, 
                    state->rho[i], 
                    state->ux[i], 
                    state->uy[i], 
                    state->barrier[i] ? 1 : 0);
        }
    }
    fclose(f);
    printf("Snapshot guardado: %s\n", filename);
}
#include "raylib.h"
#include "state.h"
#include "solver.h"
#include "renderer.h"
#include "analysis.h" 
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

SimulationState state;
RenderContext ctx;

bool simulation_running = false; // Control de estado de la simulación
int time_step_counter = 0; // Contador global de pasos
double solver_accumulated_time = 0.0; // Acumulador de tiempo de procesamiento

// Input buffer para Omega
char omega_str[32] = "1.80"; 
int omega_char_count = 4;

// Input buffer para Velocity
char velocity_str[32] = "0.06";
int velocity_char_count = 4;

// Modos de edición: 0=Ninguno, 1=Omega, 2=Velocity, 3=Snapshot
int edit_mode = 0;

// Input buffer para Snapshot Period
char snapshot_str[32] = "1000";
int snapshot_char_count = 4;
int snapshot_period = 1000;

void ResetBarriers(SimulationState *state) {
    for(int i=0; i<GRID_W*GRID_H; i++) {
        state->barrier[i] = false;
    }
}

void InitScenario(SimulationState *state, int type) {
    ResetBarriers(state);
    int cx = GRID_W / 3; // Un poco a la izquierda
    int cy = GRID_H / 2;

    if (type == 1) { // Círculo
        int r = GRID_H / 8;
        for(int y=0; y<GRID_H; y++) {
            for(int x=0; x<GRID_W; x++) {
                if ((x-cx)*(x-cx) + (y-cy)*(y-cy) <= r*r) {
                    state->barrier[idx(x,y)] = true;
                }
            }
        }
    } else if (type == 2) { // Cuadrado
        int r = GRID_H / 8;
        for(int y=cy-r; y<=cy+r; y++) {
            for(int x=cx-r; x<=cx+r; x++) {
                if(x>=0 && x<GRID_W && y>=0 && y<GRID_H)
                    state->barrier[idx(x,y)] = true;
            }
        }
    } else if (type == 3) { // Pared Vertical
        int w = 10;
        int h = GRID_H / 2;
        for(int y=cy-h/2; y<=cy+h/2; y++) {
            for(int x=cx; x<cx+w; x++) {
                 if(x>=0 && x<GRID_W && y>=0 && y<GRID_H)
                    state->barrier[idx(x,y)] = true;
            }
        }
    }
}

void UpdateDrawFrame(void) {
    // 1. Input (Interacción del usuario)
    Renderer_HandleInput(&state);
    
    // Control de inicio de simulación
    if (!simulation_running) {
        // Toggle Edición
        if (IsKeyPressed(KEY_O)) {
            if (edit_mode == 1) edit_mode = 0; // Salir de Omega
            else edit_mode = 1; // Entrar a Omega
        }
        if (IsKeyPressed(KEY_V)) {
            if (edit_mode == 2) edit_mode = 0; // Salir de Velocidad
            else edit_mode = 2; // Entrar a Velocidad
        }
        if (IsKeyPressed(KEY_S)) {
            if (edit_mode == 3) edit_mode = 0; // Salir de Snapshot
            else edit_mode = 3; // Entrar a Snapshot
        }

        if (edit_mode == 1) {
            // -- EDITAR OMEGA --
            int key = GetCharPressed();
            while (key > 0) {
                if (((key >= 48 && key <= 57) || key == 46) && (omega_char_count < 10)) {
                    omega_str[omega_char_count] = (char)key;
                    omega_str[omega_char_count+1] = '\0';
                    omega_char_count++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (omega_char_count > 0) {
                    omega_char_count--;
                    omega_str[omega_char_count] = '\0';
                }
            }
            // Actualizar Omega
            float val = (float)atof(omega_str);
            if (val < 0.1f) val = 0.1f;
            if (val > 1.99f) val = 1.99f;
            state.omega = val;

        } else if (edit_mode == 2) {
             // -- EDITAR VELOCIDAD --
            int key = GetCharPressed();
            while (key > 0) {
                if (((key >= 48 && key <= 57) || key == 46) && (velocity_char_count < 10)) {
                    velocity_str[velocity_char_count] = (char)key;
                    velocity_str[velocity_char_count+1] = '\0';
                    velocity_char_count++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (velocity_char_count > 0) {
                    velocity_char_count--;
                    velocity_str[velocity_char_count] = '\0';
                }
            }
            // Actualizar Velocidad
            float val = (float)atof(velocity_str);
            if (val < 0.0f) val = 0.0f;
            if (val > 0.5f) val = 0.5f; // Limite razonable
            state.inlet_velocity = val;
            
        } else if (edit_mode == 3) {
             // -- EDITAR SNAPSHOT --
            int key = GetCharPressed();
            while (key > 0) {
                // Solo números para snapshot (sin punto decimal)
                if ((key >= 48 && key <= 57) && (snapshot_char_count < 10)) {
                    snapshot_str[snapshot_char_count] = (char)key;
                    snapshot_str[snapshot_char_count+1] = '\0';
                    snapshot_char_count++;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (snapshot_char_count > 0) {
                    snapshot_char_count--;
                    snapshot_str[snapshot_char_count] = '\0';
                }
            }
            // Actualizar Snapshot Period
            int val = atoi(snapshot_str);
            if (val < 1) val = 1;
            snapshot_period = val;
            
        } else {
            // -- MODO NORMAL --
            if (IsKeyPressed(KEY_ENTER)) simulation_running = true;
            
            // Escenarios
            if (IsKeyPressed(KEY_ONE)) InitScenario(&state, 1); 
            if (IsKeyPressed(KEY_TWO)) InitScenario(&state, 2); 
            if (IsKeyPressed(KEY_THREE)) InitScenario(&state, 3); 
            if (IsKeyPressed(KEY_C)) ResetBarriers(&state);     
        }
    }

    // 2. Física y Análisis
    if (simulation_running) {
        // Ejecutamos varios pasos de física (ej: 4) por cada frame visual
        // para que el fluido se mueva más rápido visualmente.
        int steps_per_frame = 4;

        for(int i=0; i<steps_per_frame; i++) {
            double t0 = GetTime();
            Solver_Step(&state);     // Avanza la física
            double t1 = GetTime();
            solver_accumulated_time += (t1 - t0);

            time_step_counter++;     // Cuenta el paso
            
            if (time_step_counter % 1000 == 0) {
                Analysis_LogPerformance(time_step_counter, solver_accumulated_time);
                solver_accumulated_time = 0.0;
            }

            // --- BLOQUE DE GUARDADO DE DATOS ---
            
            // A. Métricas Globales (Energía/Masa) cada 100 pasos
            // (Esto es ligero, se puede hacer frecuente)
            if (time_step_counter % 100 == 0) {
                Analysis_ComputeAndSave(&state, time_step_counter);
            }

            // B. Snapshots Completos (Archivos grandes) cada 'snapshot_period' pasos
            if (time_step_counter % snapshot_period == 0) {
                Analysis_SaveSnapshot(&state, time_step_counter);
            }
        }
    }

    // 3. Render (Visualización)
    BeginDrawing();
        ClearBackground(BLACK);
        Renderer_Draw(&ctx, &state);
        
        // Información visual extra
        DrawText(TextFormat("Step: %d", time_step_counter), 10, 50, 10, GREEN);
        
        if (!simulation_running) {
            DrawText("PAUSED - PRESS ENTER TO START", 10, 70, 20, YELLOW);
            
            Color omegaColor = (edit_mode == 1) ? RED : WHITE;
            DrawText(TextFormat("Omega: %s", omega_str), 10, 95, 20, omegaColor);
            
            Color velColor = (edit_mode == 2) ? RED : WHITE;
            DrawText(TextFormat("Velocity: %s", velocity_str), 10, 120, 20, velColor);

            Color snapColor = (edit_mode == 3) ? RED : WHITE;
            DrawText(TextFormat("Snapshot Every: %s", snapshot_str), 10, 145, 20, snapColor);

            if (edit_mode != 0) {
                DrawText("[EDITING] Type Value - Press key again to Save", 200, 95, 10, RED);
            } else {
                DrawText("Press 'O' for Omega, 'V' for Velocity, 'S' for Snapshot", 200, 95, 10, GRAY);
            }

            DrawText("Presets: [1] Círculo  [2] Cuadrado  [3] Pared  [C] Limpiar", 10, 175, 10, GRAY);
            DrawText("Draw with Mouse Left Click", 10, 190, 10, GRAY);
        } else {
            if (time_step_counter % snapshot_period < 60) {
                DrawText("GUARDANDO SNAPSHOT...", 10, 65, 10, RED);
            }
        }
    EndDrawing();
}

int main(void) {
    InitWindow(800, 800, "Simulador de Fluidos LBM + Analisis");

    Solver_Init(&state);
    Renderer_Init(&ctx);
    
    // Inicializar el archivo CSV (escribir encabezados)
    Analysis_Init(); 
    Analysis_InitPerformanceLog(); 

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    // Limpieza de memoria
    Solver_Cleanup(&state);
    Renderer_Cleanup(&ctx);
    CloseWindow();
    return 0;
}
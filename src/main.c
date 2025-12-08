#include "raylib.h"
#include "state.h"
#include "solver.h"
#include "renderer.h"
#include "analysis.h" 
#include <stdbool.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

SimulationState state;
RenderContext ctx;

bool simulation_running = false; // Control de estado de la simulación
int time_step_counter = 0; // Contador global de pasos
double solver_accumulated_time = 0.0; // Acumulador de tiempo de procesamiento

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
        if (IsKeyPressed(KEY_ENTER)) simulation_running = true;
        
        // Escenarios
        if (IsKeyPressed(KEY_ONE)) InitScenario(&state, 1); // Círculo
        if (IsKeyPressed(KEY_TWO)) InitScenario(&state, 2); // Cuadrado
        if (IsKeyPressed(KEY_THREE)) InitScenario(&state, 3); // Pared
        if (IsKeyPressed(KEY_C)) ResetBarriers(&state);     // Limpiar

        // Control de Omega
        if (IsKeyPressed(KEY_UP)) {
            state.omega += 0.01f;
            if (state.omega > 1.95f) state.omega = 1.95f;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            state.omega -= 0.01f;
            if (state.omega < 0.5f) state.omega = 0.5f;
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

            // B. Snapshots Completos (Archivos grandes) cada 1000 pasos
            // (Esto es pesado, hazlo menos frecuente)
            if (time_step_counter % 1000 == 0) {
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
            DrawText(TextFormat("Omega: %.2f (UP/DOWN to change)", state.omega), 10, 95, 20, WHITE);
            DrawText("Presets: [1] Círculo  [2] Cuadrado  [3] Pared  [C] Limpiar", 10, 120, 10, GRAY);
            DrawText("Draw with Mouse Left Click", 10, 135, 10, GRAY);
        } else {
            if (time_step_counter % 1000 < 60) {
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
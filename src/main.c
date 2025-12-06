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

void UpdateDrawFrame(void) {
    // 1. Input (Interacción del usuario)
    Renderer_HandleInput(&state);
    
    // Control de inicio de simulación
    if (!simulation_running) {
        if (IsKeyPressed(KEY_ENTER)) {
            simulation_running = true;
        }
    }

    // 2. Física y Análisis
    if (simulation_running) {
        // Ejecutamos varios pasos de física (ej: 4) por cada frame visual
        // para que el fluido se mueva más rápido visualmente.
        int steps_per_frame = 4;

        for(int i=0; i<steps_per_frame; i++) {
            Solver_Step(&state);     // Avanza la física
            time_step_counter++;     // Cuenta el paso

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
        } else {
            if (time_step_counter % 1000 < 60) {
                DrawText("GUARDANDO SNAPSHOT...", 10, 65, 10, RED);
            }
        }
    EndDrawing();
}

int main(void) {
    InitWindow(800, 400, "Simulador de Fluidos LBM + Analisis");

    Solver_Init(&state);
    Renderer_Init(&ctx);
    
    // Inicializar el archivo CSV (escribir encabezados)
    Analysis_Init(); 

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
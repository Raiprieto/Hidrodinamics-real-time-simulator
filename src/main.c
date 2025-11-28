#include "raylib.h"
#include "state.h"
#include "solver.h"
#include "renderer.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

SimulationState state;
RenderContext ctx;

void UpdateDrawFrame(void) {
    // 1. Input
    Renderer_HandleInput(&state);
    
    // 2. Física (Varias pasadas por frame para más velocidad de fluido)
    for(int i=0; i<4; i++) Solver_Step(&state);

    // 3. Render
    BeginDrawing();
        ClearBackground(BLACK);
        Renderer_Draw(&ctx, &state);
    EndDrawing();
}

int main(void) {
    InitWindow(800, 400, "Simulador de Fluidos LBM");

    Solver_Init(&state);
    Renderer_Init(&ctx);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    Solver_Cleanup(&state);
    Renderer_Cleanup(&ctx);
    CloseWindow();
    return 0;
}
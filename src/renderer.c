#include "renderer.h"
#include <stdlib.h>
#include <math.h>

void Renderer_Init(RenderContext *ctx) {
    // Imagen en CPU
    ctx->image = GenImageColor(GRID_W, GRID_H, BLACK);
    // Textura en GPU
    ctx->texture = LoadTextureFromImage(ctx->image);
    // Puntero directo para escribir rápido
    ctx->pixels = (Color*)ctx->image.data;
}

void Renderer_HandleInput(SimulationState *state) {
    // Dibujar paredes con clic izquierdo
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        
        // Escalar mouse a coordenadas de grilla
        float scaleX = (float)GetScreenWidth() / GRID_W;
        float scaleY = (float)GetScreenHeight() / GRID_H;
        
        int gx = (int)(mouse.x / scaleX);
        int gy = (int)(mouse.y / scaleY);
        
        // Pincel de radio 2
        for(int dy=-1; dy<=1; dy++) {
            for(int dx=-1; dx<=1; dx++) {
                int nx = gx + dx;
                int ny = gy + dy;
                if(nx >=0 && nx < GRID_W && ny >=0 && ny < GRID_H) {
                    state->barrier[idx(nx, ny)] = true;
                    // Resetear velocidad en la pared
                    state->ux[idx(nx, ny)] = 0;
                    state->uy[idx(nx, ny)] = 0;
                }
            }
        }
    }
}

void Renderer_Draw(RenderContext *ctx, SimulationState *state) {
    int N = GRID_W * GRID_H;

    for (int i = 0; i < N; i++) {
        if (state->barrier[i]) {
            ctx->pixels[i] = (Color){255, 100, 100, 255}; // Obstáculo Rojo
        } else {
            // Visualizar Velocidad (Magnitud)
            float vx = state->ux[i];
            float vy = state->uy[i];
            float speed = sqrtf(vx*vx + vy*vy);
            
            // Mapear velocidad a color (Azul lento -> Verde rápido)
            unsigned char val = (unsigned char)(fminf(speed * 2000.0f, 255.0f));
            ctx->pixels[i] = (Color){0, val, val/2 + 50, 255};
        }
    }

    UpdateTexture(ctx->texture, ctx->pixels);
    
    // Dibujar estirado
    DrawTexturePro(ctx->texture, 
        (Rectangle){0, 0, GRID_W, GRID_H},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0,0}, 0.0f, WHITE);
        
    DrawText("Click Izquierdo: Dibujar Pared", 10, 10, 20, WHITE);
    DrawFPS(10, 30);
}

void Renderer_Cleanup(RenderContext *ctx) {
    UnloadTexture(ctx->texture);
    UnloadImage(ctx->image);
}
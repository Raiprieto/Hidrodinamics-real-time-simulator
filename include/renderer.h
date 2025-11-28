#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "state.h"

typedef struct {
    Texture2D texture;
    Image image;
    Color *pixels;
} RenderContext;

void Renderer_Init(RenderContext *ctx);
void Renderer_HandleInput(SimulationState *state);
void Renderer_Draw(RenderContext *ctx, SimulationState *state);
void Renderer_Cleanup(RenderContext *ctx);

#endif
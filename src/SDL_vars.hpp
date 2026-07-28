#pragma once
#include "SDL3/SDL_render.h"
#include <cstdint>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
const static int SDL_SCALE = 25;
const static int SDL_WIDTH = 64 * SDL_SCALE;
const static int SDL_HEIGHT = 32 * SDL_SCALE;

static bool color = true;
static int i = 0;
static int j = 0;

enum Color {
    BLACK = false,
    WHITE = true
};

inline void set_renderer_color(Color color) {
    switch (color) {
        case BLACK: SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); break;
        case WHITE: SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); break;
    };
}

constexpr static auto draw = [](uint8_t w, uint8_t h, Color color) {
    const int sdl_w = w * SDL_SCALE;
    const int sdl_h = h * SDL_SCALE;
    
    set_renderer_color(color);
    for (int i = 0; i < SDL_SCALE; i++) {
        for (int j = 0; j < SDL_SCALE; j++) {
            SDL_RenderPoint(renderer, sdl_w + i, sdl_h + j);
        }
    }
};

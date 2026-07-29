#pragma once
#include "SDL3/SDL_render.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

const static int SDL_SCALE = 25;
const static int SDL_WIDTH = 64;
const static int SDL_HEIGHT = 32;

enum HexColor : uint32_t {
    HEX_COLOR_WHITE = 0xFFFFFFFF,
    HEX_COLOR_BLACK = 0x000000FF,
};

template <typename T, typename Derived>
void debug_pixel(T w, T h, const Derived& color) {
    auto c = color.color();
    std::cout << "(" 
        << static_cast<int>(w) << ", " << static_cast<int>(h) << "): Palette enum #"
        << std::hex << static_cast<uint32_t>(c) << std::dec << "\n";
}

template <typename T, typename DerivedArr, typename P>
void draw_canvas(T full_w, T full_h, const DerivedArr& arr, const P& color_encoder) {
    std::array<uint32_t, SDL_WIDTH * SDL_HEIGHT> pixels;

    std::transform(arr.begin(), arr.end(), pixels.begin(), 
        [color_encoder](const auto& color_px) {
            return color_encoder.hex(color_px.color());
        });

    SDL_UpdateTexture(texture, NULL, &pixels, SDL_WIDTH * sizeof(uint32_t));
}
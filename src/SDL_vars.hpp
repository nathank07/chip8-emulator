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

template <typename Derived>
struct PixelState {
    template <typename F>
    auto with_color(F&& do_with_color) const {
        const auto& self = static_cast<const Derived&>(*this);
        return do_with_color(self.color());
    }
};

enum HexColor : uint32_t {
    HEX_COLOR_WHITE = 0xFFFFFFFF,
    HEX_COLOR_BLACK = 0x000000FF,
};

template <typename Derived, typename Color>
struct Palette {
    uint32_t hex(Color color) const {
        const auto& self = static_cast<const Derived&>(*this);
        return self.convert_color_to_hex(color);
    }
};

template <typename T, typename Derived>
void debug_pixel(T w, T h, const PixelState<Derived>& color) {
    color.with_color([w, h](auto color) {
        std::cout << "(" 
            << static_cast<int>(w) << ", " << static_cast<int>(h) << "): Palette enum #"
            << std::hex << static_cast<uint32_t>(color) << std::dec << "\n";
    });
}

template <typename T, typename DerivedArr, typename F>
void draw_canvas(T full_w, T full_h, const DerivedArr& arr, const F& color_encoder) {
    std::array<uint32_t, SDL_WIDTH * SDL_HEIGHT> pixels;

    std::transform(arr.begin(), arr.end(), pixels.begin(), 
        [color_encoder](const auto& color_px) {
            return color_px.with_color([color_encoder](auto color) {
                return color_encoder.hex(color);
            });
        });

    SDL_UpdateTexture(texture, NULL, &pixels, SDL_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
}
#pragma once
#include "SDL3/SDL_pixels.h"
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
        return do_with_color(self.get_r(), self.get_g(), self.get_b(), self.get_a());
    }
};

template <typename T, typename Derived>
void debug_pixel(T w, T h, const PixelState<Derived>& color) {
    color.with_color([w, h](auto r, auto g, auto b, auto a) {
        std::cout << "(" 
            << static_cast<int>(w) << ", " << static_cast<int>(h) << "): "
            << static_cast<int>(r) << "r " << static_cast<int>(g) << "g "
            << static_cast<int>(b) << "b " << static_cast<int>(a) << "a\n";
    });
}

template <typename T, typename Derived>
void draw_pixel(T w, T h, const PixelState<Derived>& color) {
    color.with_color([w, h](auto r, auto g, auto b, auto a) {
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderPoint(renderer, w, h);
    });
}

template <typename T, typename DerivedArr>
void draw_canvas(T full_w, T full_h, DerivedArr arr) {
    std::array<uint32_t, SDL_WIDTH * SDL_HEIGHT> pixels;

    std::transform(arr.begin(), arr.end(), pixels.begin(), [](const auto& color_px) {
        return color_px.with_color([](auto r, auto g, auto b, auto a) {
            return (r << 24) | (g << 16) | (b << 8) | a;
        });
    });

    SDL_UpdateTexture(texture, NULL, &pixels, SDL_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
}
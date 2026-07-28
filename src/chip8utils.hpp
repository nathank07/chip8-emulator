#pragma once
#include <stdint.h>
#include "SDL_vars.hpp"

struct Chip8PixelState : PixelState<Chip8PixelState> {

    constexpr static bool ON  = true;
    constexpr static bool OFF = false;

    Chip8PixelState() = default;
    Chip8PixelState(bool enabled) : enabled(enabled) {}
    
    bool operator==(const Chip8PixelState& other) {
        return enabled == other.enabled;
    }

    bool operator!=(const Chip8PixelState& other) {
        return !(*this == other);
    }

    bool enabled = false;

    uint8_t get_r() const {
        return enabled ? 255 : 0;
    }

    uint8_t get_g() const {
        return enabled ? 255 : 0;
    }

    uint8_t get_b() const {
        return enabled ? 255 : 0;
    }

    uint8_t get_a() const {
        return 255;
    }
};

struct Chip8Display {

    using Color = Chip8PixelState;

    constexpr static uint8_t DISPLAY_WIDTH = SDL_WIDTH;
    constexpr static uint8_t DISPLAY_HEIGHT = SDL_HEIGHT;

    constexpr static auto render = 
        [](std::array<Color, DISPLAY_WIDTH * DISPLAY_HEIGHT> canvas) {
            draw_canvas(DISPLAY_WIDTH, DISPLAY_HEIGHT, canvas);
        };

    constexpr static auto debug_pixel_draw = 
        [](uint8_t w, uint8_t h, Color c) {
            debug_pixel(w, h, c);
        };
};
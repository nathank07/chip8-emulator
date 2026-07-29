#pragma once
#include <stdint.h>
#include "SDL_vars.hpp"

struct Chip8PixelState : PixelState<Chip8PixelState> {

    bool enabled = false;

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

    bool color() const {
        return enabled;
    }

};

struct Chip8Palette : Palette<Chip8Palette, bool> {

    using Color = bool;

    uint32_t foreground;
    uint32_t background;

    uint32_t convert_color_to_hex(Color color) const {
        return color ? foreground : background;
    }

    Chip8Palette(
        uint32_t foreground = HEX_COLOR_WHITE, 
        uint32_t background = HEX_COLOR_BLACK
    )
        : foreground(foreground), background(background) {} 
};

struct Chip8Display {

    using Color = Chip8PixelState;
    using Palette = Chip8Palette;

    constexpr static uint8_t DISPLAY_WIDTH = SDL_WIDTH;
    constexpr static uint8_t DISPLAY_HEIGHT = SDL_HEIGHT;

    constexpr static auto render = [](
        const std::array<Color, DISPLAY_WIDTH * DISPLAY_HEIGHT>& canvas,
        const Palette& palette
    ) {
        draw_canvas(DISPLAY_WIDTH, DISPLAY_HEIGHT, canvas, palette);
    };

    constexpr static auto debug_pixel_draw = 
        [](uint8_t w, uint8_t h, Color c) {
            debug_pixel(w, h, c);
        };
};
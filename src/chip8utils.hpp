#pragma once
#include <stdint.h>
#include "SDL_vars.hpp"

struct Chip8PixelState : PixelState<Chip8PixelState> {

    bool enabled = false;

    constexpr static bool ON  = true;
    constexpr static bool OFF = false;

    enum class Chip8Color : std::uint32_t {
        BACKGROUND,
        FOREGROUND
    };

    Chip8PixelState() = default;
    Chip8PixelState(bool enabled) : enabled(enabled) {}
    
    bool operator==(const Chip8PixelState& other) {
        return enabled == other.enabled;
    }

    bool operator!=(const Chip8PixelState& other) {
        return !(*this == other);
    }

    Chip8Color color() const {
        return enabled ? Chip8Color::FOREGROUND : Chip8Color::BACKGROUND;
    }

};

struct Chip8Palette {

    using Color = typename Chip8PixelState::Chip8Color;

    static constexpr auto white_on_black = [](Color color) -> uint32_t {
        switch (color) {
            case Color::BACKGROUND: return 0x000000FF;
            case Color::FOREGROUND: return 0xFFFFFFFF;
        }
    };

    static constexpr auto black_on_white = [](Color color) -> uint32_t {
        switch (color) {
            case Color::BACKGROUND: return 0xFFFFFFFF;
            case Color::FOREGROUND: return 0x000000FF;
        }
    };
};

struct Chip8Display {

    using Color = Chip8PixelState;

    constexpr static uint8_t DISPLAY_WIDTH = SDL_WIDTH;
    constexpr static uint8_t DISPLAY_HEIGHT = SDL_HEIGHT;

    constexpr static auto render = 
        [](const std::array<Color, DISPLAY_WIDTH * DISPLAY_HEIGHT>& canvas) {
            draw_canvas(
                DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                canvas, Chip8Palette::white_on_black
            );
        };

    constexpr static auto debug_pixel_draw = 
        [](uint8_t w, uint8_t h, Color c) {
            debug_pixel(w, h, c);
        };
};
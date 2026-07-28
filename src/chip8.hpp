#pragma once
#include "display.hpp"
#include "memory.hpp"
#include "chip8utils.hpp"


struct Chip8 {

    Display<Chip8Display> display;
    Memory memory;

    void draw_white(uint8_t w, uint8_t h) {
        display.draw_batch([w, h](auto& d) {
            d.draw_pixel(w, h, Chip8PixelState::ON);
        });
    }

    void draw_black(uint8_t w, uint8_t h) {
        display.draw_batch([w, h](auto& d) {
            d.draw_pixel(w, h, Chip8PixelState::OFF);
        });
    }
};
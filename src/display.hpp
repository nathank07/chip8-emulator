#pragma once
#include <array>
#include <cassert>
#include <cstdint>

template <typename T>
struct Display {

    using Color = typename T::Color;
    using Palette = typename T::Palette;

    std::array<Color, T::DISPLAY_WIDTH * T::DISPLAY_HEIGHT> display_values;
    Palette palette{};

    Color& at(uint8_t x, uint8_t y) {
        assert(x + (T::DISPLAY_WIDTH * y) < display_values.size());
        return display_values[x + (T::DISPLAY_WIDTH * y)];
    }

    void clear_with(Color value) {
        display_values.fill(value);
        T::render(display_values, palette);
    }

    template <typename F>
    auto with_pixel(uint8_t x, uint8_t y, F&& do_with_pixel) {
        return do_with_pixel(at(x, y));
    }

    template <typename F>
    void with_palette(F&& do_with_palette) {
        do_with_palette(palette);
        T::render(display_values, palette);
    }

    template <typename F>
    void draw_batch(F&& draw_batch_f) {
        draw_batch_f(*this);
        T::render(display_values, palette);
    }
};
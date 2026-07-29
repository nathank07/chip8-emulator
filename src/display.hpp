#include <array>
#include <cstdint>

template <typename T>
struct Display {

    using Color = typename T::Color;
    using Palette = typename T::Palette;

    std::array<Color, T::DISPLAY_WIDTH * T::DISPLAY_HEIGHT> display_values;
    Palette palette{};

    Color& at(uint8_t width, uint8_t height) {
        return display_values[width + (T::DISPLAY_WIDTH * height)];
    }

    void draw_pixel(uint8_t width, uint8_t height, Color value) {
        at(width, height) = value;
        T::debug_pixel_draw(width, height, value);
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
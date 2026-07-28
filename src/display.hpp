#include <array>
#include <cstdint>

template <typename T>
struct Display {

    using Color = typename T::Color;

    std::array<Color, T::DISPLAY_WIDTH * T::DISPLAY_HEIGHT> display_values;

    Color& at(uint8_t width, uint8_t height) {
        return display_values[width + (T::DISPLAY_WIDTH * height)];
    }

    void draw_pixel(uint8_t width, uint8_t height, Color value) {
        at(width, height) = value;
        T::debug_pixel_draw(width, height, value);
    }

    template <typename F>
    void draw_batch(F&& draw_batch_f) {
        draw_batch_f(*this);
        T::render(display_values);
    }
};
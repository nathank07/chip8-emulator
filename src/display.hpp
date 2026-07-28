#include <array>
#include <cstdint>

template <typename Color, typename F>
struct Display {
    const static uint8_t DISPLAY_WIDTH = 64;
    const static uint8_t DISPLAY_HEIGHT = 32;

    std::array<Color, DISPLAY_WIDTH * DISPLAY_HEIGHT> display_values;
    F draw;

    Display(F&& draw_width_height) : draw(draw_width_height) {}

    Color& at(uint8_t width, uint8_t height) {
        return display_values[width + (DISPLAY_WIDTH * height)];
    }

    void set_px_value(uint8_t width, uint8_t height, Color value) {
        at(width, height) = value;
        draw(width, height, value);
    }
};
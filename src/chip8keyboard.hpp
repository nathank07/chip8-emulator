#pragma once

#include <array>
#include <cassert>
#include <cstdint>

struct Chip8Keyboard {
private:
    std::array<bool, 16> keyboard{};
    std::array<bool, 16> shadow{};
public:
    void update_keyup_events() {
        shadow = keyboard;
    }

    void keydown_event(uint8_t key) {
        assert(key <= 0xF);
        keyboard[key] = true;
    }

    void keyup_event(uint8_t key) {
        assert(key <= 0xF);
        keyboard[key] = false;
    }

    bool get_key(uint8_t key) const {
        assert(key <= 0xF);
        return keyboard[key];
    }

    bool recent_keyup(uint8_t key) const {
        assert(key <= 0xF);
        return !keyboard[key] && shadow[key];
    }
};
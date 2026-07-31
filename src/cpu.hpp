#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <stack>

struct CPU {
    uint16_t program_counter = 0x200;
    uint16_t index_register = 0;
    uint8_t delay_timer = 60;
    uint8_t sound_timer = 60;
    std::array<uint8_t, 16> registers{};
    std::stack<uint16_t> function_pointers;

    uint8_t& reg(uint8_t r) {
        assert(r <= 15);
        return registers[r];
    }
};
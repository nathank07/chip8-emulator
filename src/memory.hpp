#pragma once

#include <array>
#include <cstdint>

struct Memory {
    std::array<uint8_t, 4096> memory;

    uint16_t fetch(uint16_t index) {
        return (memory[index] << 8 | memory[index + 1]);
    }
};
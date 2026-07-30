#pragma once

#include <array>
#include <cassert>
#include <cstdint>

struct Memory {
    std::array<uint8_t, 4096> memory;

    uint16_t fetch(uint16_t index) {
        return (memory[index] << 8 | memory[index + 1]);
    }

    uint8_t& operator[](std::size_t idx) {
        assert(idx < memory.size());
        return memory[idx];
    }
};
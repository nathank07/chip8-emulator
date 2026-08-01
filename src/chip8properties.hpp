#pragma once

#include <cmath>
#include <cstdint>
#include "utils.hpp"
#include "instructions.hpp"

struct Chip8Properties {

    using Instruction = Chip8ISA::Instruction; 

    uint16_t instructions_per_second = 700;
    uint16_t ticks_per_second = 60;

    bool jump_offset_uses_reg = false;
    bool add_index_sets_vf = true;
    bool shift_sets_vf = false;
    bool store_load_increments_idx = false;

    uint64_t instructions_per_second_ns() {
        return 1'000'000'000ULL / instructions_per_second;
    }

    uint64_t ticks_per_second_ns() {
        return 1'000'000'000ULL / ticks_per_second;
    }
    
    uint8_t set_shift(uint8_t dst, uint8_t src) {
        return shift_sets_vf ? dst : src;
    }

    uint16_t set_store_load_idx(uint16_t init_size, uint8_t size) {
        return store_load_increments_idx ? 
            init_size + size :
            size;
    }

    uint16_t skip(bool on_cond) {
        // Could also be 4 : 2, but advance_ip_with
        // would need to be changed
        return on_cond ? 2 : 0;
    }

    uint16_t advance_ip_with(const Instruction& instruction) {
        return std::visit(overloads {
            [&](const Chip8ISA::ReturnSubroutine&) { return 0; },
            [&](const Chip8ISA::CallSubroutine&) { return 0; },
            [&](const Chip8ISA::JumpOffset&) { return 0; },
            [&](const Chip8ISA::GetKey&) { return 0; },
            [&](const Chip8ISA::Jump&) { return 0; },
            [&](const auto&)           { return 2; },
        }, instruction);
    }
};
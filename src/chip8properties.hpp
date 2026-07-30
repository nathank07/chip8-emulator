#pragma once

#include <cstdint>
#include "utils.hpp"
#include "instructions.hpp"

struct Chip8Properties {

    using Instruction = Chip8ISA::Instruction; 

    uint16_t advance_ip_with(const Instruction& instruction) {
        return std::visit(overloads {
            [&](const Chip8ISA::Jump&) { return 0; },
            [&](const auto&)           { return 4; },
        }, instruction);
    }
};
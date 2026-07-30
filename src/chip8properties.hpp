#pragma once

#include <cstdint>
#include "utils.hpp"
#include "instructions.hpp"

struct Chip8Properties {

    uint16_t skip(bool on_cond) {
        // Could also be 4 : 2, but advance_ip_with
        // would need to be changed
        return on_cond ? 2 : 0;
    }

    template <typename ISA, typename Instruction>
    uint16_t advance_ip_with(const Instruction& instruction) {
        return std::visit(overloads {
            [&](const ISA::ReturnSubroutine&) { return 0; },
            [&](const ISA::CallSubroutine&) { return 0; },
            [&](const ISA::Jump&) { return 0; },
            [&](const auto&)           { return 2; },
        }, instruction);
    }
};
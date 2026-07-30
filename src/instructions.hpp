#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <variant>

namespace Chip8ISA {

    struct Operand4_reg {
        const uint8_t r;
        Operand4_reg() = delete;
        Operand4_reg(uint8_t n) : r(n) {
            assert((n & 0xF0) == 0x0);
        }
    };

    struct Operand4_imm {
        const uint8_t v;
        Operand4_imm() = delete;
        Operand4_imm(uint8_t n) : v(n) {
            assert((n & 0xF0) == 0x0);
        }
    };

    struct Operand8_imm {
        const uint8_t v;
        Operand8_imm(uint8_t n) : v(n) {}
    };

    struct Operand12_imm {
        const uint16_t v;
        Operand12_imm() = delete;
        Operand12_imm(uint16_t n) : v(n) {
            assert((n & 0xF000) == 0x0);
        }
    };

    struct Clear {};

    struct Jump {
        const Operand12_imm imm;
    };

    struct CallSubroutine {
        const Operand12_imm imm;
    };

    struct ReturnSubroutine {};

    struct SkipEqImm {
        const Operand4_reg reg;
        const Operand8_imm imm;
    };

    struct SkipNeqImm {
        const Operand4_reg reg;
        const Operand8_imm imm;
    };

    struct SkipEqReg {
        const Operand4_reg r1;
        const Operand4_reg r2;
    };

    struct SkipNeqReg {
        const Operand4_reg r1;
        const Operand4_reg r2;
    };

    struct SetImm {
        const Operand4_reg reg;
        const Operand8_imm imm; 
    };

    struct SetReg {
        const Operand4_reg dst_reg;
        const Operand4_reg src_reg;
    };

    struct SetIndexReg {
        const Operand12_imm imm;
    };

    struct Add {
        const Operand4_reg reg;
        const Operand8_imm imm;
    };

    struct Display {
        const Operand4_reg x_coordinate;
        const Operand4_reg y_coordinate;
        const Operand4_imm rows;
    };

    using Instruction = std::variant<
        Clear, Jump, CallSubroutine, ReturnSubroutine, SkipEqImm, SkipNeqImm,
        SkipEqReg, SkipNeqReg, SetImm, SetReg, SetIndexReg, Add, Display
    >;

   
    struct UnimplementedInstruction {
        uint16_t bytes;
    };

    using InstructionError = UnimplementedInstruction;

    inline std::expected<Chip8ISA::Instruction, InstructionError> decode(uint16_t bytes) {
        
        uint8_t opcode = bytes >> 12;

        constexpr static auto _xxx = [](uint16_t b) -> uint16_t {
            return b & 0xFFF;
        };

        constexpr static auto __xx = [](uint16_t b) -> uint8_t {
            return b & 0xFF;
        };

        constexpr static auto ___x = [](uint16_t b) -> uint8_t {
            return b & 0xF;
        };

        constexpr static auto _x__ = [](uint16_t b) -> uint8_t {
            return b >> 8 & 0xF;
        };

        constexpr static auto __x_ = [](uint16_t b) -> uint8_t {
            return b >> 4 & 0xF;
        };

        switch (opcode) {

            case 0x0: {
                if ((bytes & 0xFFF) == 0x0E0) 
                    return Clear{};
                
                if ((bytes & 0xFFF) == 0x0EE)
                    return ReturnSubroutine{};

                break;
            }

            case 0x1: {
                return Jump{ .imm = _xxx(bytes) };
            }

            case 0x6: {
                return SetImm{
                    .reg = _x__(bytes),
                    .imm = __xx(bytes)
                };
            }

            case 0x7: {
                return Add{
                    .reg = _x__(bytes),
                    .imm = __xx(bytes)
                };
            }

            case 0xA: {
                return SetIndexReg{ .imm = _xxx(bytes) };
            }

            case 0xD: {
                return Display{
                    .x_coordinate = _x__(bytes),
                    .y_coordinate = __x_(bytes),
                    .rows = ___x(bytes)
                };
            }
        }

        return std::unexpected<UnimplementedInstruction>{bytes};
    }

    
}
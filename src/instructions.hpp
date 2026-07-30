#pragma once

#include <cassert>
#include <cstdint>
#include <expected>
#include <functional>
#include <variant>


template <typename T>
struct Chip8ISA {

    std::reference_wrapper<T> m;

    Chip8ISA(T& mt) : m(mt) {}

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

    
    struct Operand4_reg {
        Operand4_reg() = delete;
        Operand4_reg(uint8_t reg, T& decoder) : reg(reg), decoder(decoder) {
            assert((reg & 0xF0) == 0x0);
        }

        uint8_t v() const {
            return decoder.get().reader(reg);
        }

        int operator&() = delete;
        int operator*() = delete;

        private:
        const uint8_t reg;
        std::reference_wrapper<T> decoder;
    };

    Operand4_reg r(uint8_t reg) {
        return Operand4_reg(reg, m);
    }

    struct Operand4_reg_ptr {
        Operand4_reg_ptr() = delete;
        Operand4_reg_ptr(uint8_t reg, T& decoder) : reg(reg), decoder(decoder) {
            assert((reg & 0xF0) == 0x0);
        }

        uint8_t addr() const {
            return decoder.get().reader(reg);
        };

        uint8_t& operator*() const {
            return decoder.get().dereferencer(reg);
        };

        private:
        const uint8_t reg;
        std::reference_wrapper<T> decoder;
    };

    Operand4_reg_ptr p(uint8_t reg) {
        return Operand4_reg_ptr(reg, m);
    }

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

    struct JumpOffset {
        const Operand12_imm long_imm;
        Operand4_reg reg() { return _x__(long_imm.v); }
        Operand8_imm short_imm() { return __xx(long_imm.v); }
    };

    struct CallSubroutine {
        const Operand12_imm imm;
    };

    struct ReturnSubroutine {};

    struct SkipEqImm {
        const Operand4_reg v1;
        const Operand8_imm v2;
    };

    struct SkipNeqImm {
        const Operand4_reg v1;
        const Operand8_imm v2;
    };

    struct SkipEqReg {
        const Operand4_reg v1;
        const Operand4_reg v2;
    };

    struct SkipNeqReg {
        const Operand4_reg v1;
        const Operand4_reg v2;
    };

    struct SetImm {
        const Operand4_reg_ptr dst;
        const Operand8_imm src; 
    };

    struct SetReg {
        const Operand4_reg_ptr dst;
        const Operand4_reg_ptr src;
    };

    struct SetIndexReg {
        const Operand12_imm imm;
    };

    struct AddIndexReg {
        const Operand4_reg_ptr src;
    };

    struct Add {
        const Operand4_reg_ptr dst;
        const Operand8_imm src;
    };

    struct Sub {
        const Operand4_reg_ptr dst;
        const Operand8_imm src;
    };

    struct BitwiseOr {
        const Operand4_reg dst;
        const Operand4_reg src;
    };

    struct BitwiseAnd {
        const Operand4_reg dst;
        const Operand4_reg src;
    };

    struct BitwiseXor {
        const Operand4_reg dst;
        const Operand4_reg src;
    };

    struct ShiftLeft {
        const Operand4_reg dst;
        const Operand4_reg src;
    };

    struct ShiftRight {
        const Operand4_reg dst;
        const Operand4_reg src;
    };

    struct Random {
        const Operand4_reg dst;
        const Operand8_imm bitwise_and_with;
    };

    struct Display {
        const Operand4_reg_ptr x_coordinate;
        const Operand4_reg_ptr y_coordinate;
        const Operand4_imm rows;
    };

    struct SkipIfKeyPressed {
        const Operand4_reg key;
    };

    struct SkipIfKeyNotPressed {
        const Operand4_reg key;
    };

    struct SetRegWithTimer {
        const Operand4_reg dst; 
    };

    struct SetDelayTimer {
        const Operand4_reg dst;
    };

    struct SetSoundTimer {
        const Operand4_reg dst;
    };

    struct GetKey {
        const Operand4_reg dst;
    };

    struct SetIndexToFontAddr {
        const Operand4_reg font;
    };

    struct ConvertDecimalIntoIndexBuff {
        const Operand4_reg src;
    };

    struct LoadMemory {
        const Operand4_reg src;
    };

    struct StoreMemory {
        const Operand4_reg src;
    };

    using Instruction = std::variant<
        Clear, Jump, CallSubroutine, ReturnSubroutine, SkipEqImm, SkipNeqImm,
        SkipEqReg, SkipNeqReg, SetImm, SetReg, SetIndexReg, Add, Display
    >;

   
    struct UnimplementedInstruction {
        uint16_t bytes;
    };

    using InstructionError = UnimplementedInstruction;

    std::expected<Instruction, InstructionError> decode(uint16_t bytes) {
        
        uint8_t opcode = bytes >> 12;

        switch (opcode) {

            case 0x0: {
                if (_xxx(bytes) == 0x0E0) 
                    return Clear{};
                
                if (_xxx(bytes) == 0x0EE)
                    return ReturnSubroutine{};

                break;
            }

            case 0x1: {
                return Jump{ .imm = _xxx(bytes) };
            }

            case 0x2: {
                return CallSubroutine{ .imm = _xxx(bytes) };
            }

            case 0x3: {
                return SkipEqImm{
                    .v1 = r(_x__(bytes)),
                    .v2 =   __xx(bytes)
                };
            }

            case 0x4: {
                return SkipNeqImm{
                    .v1 = r(_x__(bytes)),
                    .v2 =   __xx(bytes)
                };
            }

            case 0x5: {

                if (___x(bytes) == 0x0)
                    return SkipEqReg{
                        .v1 = r(_x__(bytes)),
                        .v2 = r(__x_(bytes))
                    };

                break;
            }

            case 0x6: {
                return SetImm{
                    .dst = p(_x__(bytes)),
                    .src =   __xx(bytes)
                };
            }

            case 0x7: {
                return Add{
                    .dst = p(_x__(bytes)),
                    .src =   __xx(bytes)
                };
            }

            case 0x8: {
                if (___x(bytes) == 0x0)
                    return SetReg{
                        .dst = p(_x__(bytes)),
                        .src = p(__x_(bytes))
                    };

                break;
            }

            case 0x9: {

                if (___x(bytes) == 0x0)
                    return SkipNeqReg{
                        .v1 = r(_x__(bytes)),
                        .v2 = r(__x_(bytes))
                    };

                break;
            }

            case 0xA: {
                return SetIndexReg{ .imm = _xxx(bytes) };
            }

            case 0xD: {
                return Display{
                    .x_coordinate = p(_x__(bytes)),
                    .y_coordinate = p(__x_(bytes)),
                    .rows         =   ___x(bytes)
                };
            }
        }

        return std::unexpected<UnimplementedInstruction>{bytes};
    }

    
};
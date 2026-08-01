#pragma once
#include "display.hpp"
#include "memory.hpp"
#include "instructions.hpp"
#include "chip8display.hpp"
#include "cpu.hpp"
#include "chip8properties.hpp"
#include "utils.hpp"
#include <cstring>
#include <expected>
#include <random>
#include <ranges>
#include "chip8keyboard.hpp"

struct Chip8 {

    using Instruction = Chip8ISA::Instruction;

    Display<Chip8Display> display;
    CPU cpu;
    Memory mem;
    Chip8Properties props;
    Chip8Keyboard keyboard;

    template <typename T>
    bool load_rom(T arr) {
        if (arr.size() > 0xFFF - 0x200 + 1)
            return false;

        std::memcpy(mem.memory.data() + 0x200, arr.data(), arr.size());
        cpu = CPU();
        return true;
    }

    std::expected<void, Chip8ISA::InstructionError> run() {
        return Chip8ISA::decode(mem.fetch(cpu.program_counter))
            .transform([this](const Instruction& i) { execute(i); });
    }

    auto reader() {
        return [this](uint8_t reg) -> uint8_t& {
            return cpu.reg(reg);
        };
    }

    void _display(const Chip8ISA::Display& instr) {
        uint16_t x = instr.x_coordinate.v(reader()) % Chip8Display::DISPLAY_WIDTH;
        uint16_t y = instr.y_coordinate.v(reader()) % Chip8Display::DISPLAY_HEIGHT;
        auto row_count = instr.rows.v;
        uint16_t arr_idx = cpu.index_register;
        
        display.draw_batch([this, x, y, arr_idx, row_count](auto& d){

            bool vf = false;

            for (uint16_t row = 0; row < row_count; ++row) {
                uint8_t byte = mem[arr_idx + row];

                for (uint8_t col = 0; col < 8; ++col) {
                    auto cx = x + col;
                    auto cy = y + row;

                     if (cx >= Chip8Display::DISPLAY_WIDTH 
                     ||  cy >= Chip8Display::DISPLAY_HEIGHT) {
                        continue;
                     }  
                      
                    d.with_pixel(cx, cy, [&vf, byte, col](Chip8Display::Color& c) {
                        bool was_enabled = c.enabled;
                        c.enabled ^= byte >> (7 - col) & 0x1;
                        if (was_enabled && !c.enabled) vf = true;
                    });

                }
            }

            cpu.reg(0xF) = static_cast<uint8_t>(vf);
        });
    }

    uint8_t random_number() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> distrib(0, 255);
        return distrib(gen);
    }

    void execute(Instruction instruction) {

        const auto r = reader();

        std::visit(overloads{
            [this](const Chip8ISA::Clear&) { 
                display.clear_with(Chip8PixelState::OFF); 
            },
            [this](const Chip8ISA::Jump& i) { 
                cpu.program_counter = i.imm.v; 
            },
            [this, r](const Chip8ISA::JumpOffset& i) {
                cpu.program_counter = props.jump_offset_uses_reg ?
                    i.reg().v(r) + i.short_imm().v :
                    r(0x0) + i.long_imm.v;
            },
            [this](const Chip8ISA::CallSubroutine& i) {
                cpu.function_pointers.push(cpu.program_counter + 2);
                cpu.program_counter = i.imm.v;
            },
            [this](const Chip8ISA::ReturnSubroutine&) {
                const auto top = cpu.function_pointers.top();
                cpu.function_pointers.pop(); 
                cpu.program_counter = top;
            },
            [this, r](const Chip8ISA::SkipEqImm& i) {
                cpu.program_counter += props.skip(
                    i.reg.v(r) == i.imm.v
                );
            },
            [this, r](const Chip8ISA::SkipNeqImm& i) {
                cpu.program_counter += props.skip(
                    i.reg.v(r) != i.imm.v
                );
            },
            [this, r](const Chip8ISA::SkipEqReg& i) {
                cpu.program_counter += props.skip(
                    i.r1.v(r) == i.r2.v(r)
                );
            },
            [this, r](const Chip8ISA::SkipNeqReg& i) {
                cpu.program_counter += props.skip(
                    i.r1.v(r) != i.r2.v(r)
                );
            },
            [this, r](const Chip8ISA::SetImm& i) {
                i.dst.v(r) = i.src.v;
            },
            [this, r](const Chip8ISA::SetReg& i) {
                i.dst.v(r) = i.src.v(r);
            },
            [this](const Chip8ISA::SetIndexReg& i) {
                cpu.index_register = i.imm.v;
            },
            [this, r](const Chip8ISA::AddImm& i) {
                i.dst.v(r) += i.src.v;
            },
            [this, r](const Chip8ISA::AddReg& i) {
                uint16_t total = i.dst.v(r) + i.src.v(r);
                i.dst.v(r) = static_cast<uint8_t>(total);
                r(0xF) = static_cast<uint8_t>(total > 255);
            },
            [this, r](const Chip8ISA::AddIndexReg& i) {
                cpu.index_register = cpu.index_register + i.src.v(r);
                if (props.add_index_sets_vf && cpu.index_register > 0x0FFF) 
                    r(0xF) = 0x1;
            },
            [this, r](const Chip8ISA::SubRegDstLhs& i) {
                uint8_t vf = i.dst.v(r) >= i.src.v(r);
                i.dst.v(r) = i.dst.v(r) - i.src.v(r);
                r(0xF) = vf;
            },
            [this, r](const Chip8ISA::SubRegSrcLhs& i) {
                uint8_t vf = i.dst.v(r) <= i.src.v(r);
                i.dst.v(r) = i.src.v(r) - i.dst.v(r);
                r(0xF) = vf;
            },
            [this, r](const Chip8ISA::ShiftLeft& i) {
                uint8_t& byte = i.dst.v(r);
                byte = props.set_shift(byte, i.src.v(r));
                uint8_t vf = byte >> 7;
                byte <<= 1;
                r(0xF) = vf;
            },
            [this, r](const Chip8ISA::ShiftRight& i) {
                uint8_t& byte = i.dst.v(r);
                byte = props.set_shift(byte, i.src.v(r));
                uint8_t vf = byte & 0x1;
                byte >>= 1;
                r(0xF) = vf;
            },
            [this, r](const Chip8ISA::BitwiseOr& i) {
                r(0xF) = props.set_vf_after_bitwise(r(0xF));
                i.dst.v(r) |= i.src.v(r);
            },
            [this, r](const Chip8ISA::BitwiseAnd& i) {
                r(0xF) = props.set_vf_after_bitwise(r(0xF));
                i.dst.v(r) &= i.src.v(r);
            },
            [this, r](const Chip8ISA::BitwiseXor& i) {
                r(0xF) = props.set_vf_after_bitwise(r(0xF));
                i.dst.v(r) ^= i.src.v(r);
            },
            [this, r](const Chip8ISA::Random& i) {
                i.dst.v(r) = random_number() & i.bitwise_and_with.v; 
            },
            [this, r](const Chip8ISA::SetRegWithTimer& i) {
                i.dst.v(r) = cpu.delay_timer;
            },
            [this, r](const Chip8ISA::SetDelayTimer& i) {
                cpu.delay_timer = i.dst.v(r);
            },
            [this, r](const Chip8ISA::SetSoundTimer& i) {
                cpu.sound_timer = i.dst.v(r);
            },
            [this, r](const Chip8ISA::SetIndexToFontAddr& i) {
                cpu.index_register = i.src.v(r) * 5;
            },
            [this, r](const Chip8ISA::ConvertDecimalIntoIndexBuff& i) {
                uint8_t byte = i.src.v(r);
                uint16_t idx = cpu.index_register;
                mem[idx + 2] = byte % 10; byte /= 10;
                mem[idx + 1] = byte % 10; byte /= 10;
                mem[idx] = byte % 10;
            },
            [this, r](const Chip8ISA::StoreMemory& i) {
                const uint8_t literal_r = i.dst.copy(std::identity{}) + 1;
                const uint16_t idx = cpu.index_register;

                for (uint8_t i = 0; i < literal_r; ++i) 
                    mem[i + idx] = r(i); 
                
                cpu.index_register = props.set_store_load_idx(idx, literal_r);
            },
            [this, r](const Chip8ISA::LoadMemory& i) {
                const uint8_t literal_r = i.src.copy(std::identity{}) + 1;
                const uint16_t idx = cpu.index_register;

                for (uint8_t i = 0; i < literal_r; ++i) 
                    r(i) = mem[i + idx]; 
                
                cpu.index_register = props.set_store_load_idx(idx, literal_r);
            },
            [this, r](const Chip8ISA::SkipIfKeyPressed& i) {
                cpu.program_counter += keyboard.get_key(i.key.v(r)) ? 2 : 0;
            },
            [this, r](const Chip8ISA::SkipIfKeyNotPressed& i) {
                cpu.program_counter += !keyboard.get_key(i.key.v(r)) ? 2 : 0;
            },
            [this, r](const Chip8ISA::GetKey& i) {
                auto is_up = [this](int key) { return keyboard.recent_keyup(key); };
                auto keys = std::views::iota(0x0, 0x10);
                auto it = std::ranges::find_if(keys, is_up);

                if (it != keys.end()) {
                    cpu.program_counter += 2;
                    i.dst.v(r) = *it;
                }
            },
            [this](const Chip8ISA::Display& i) { _display(i); },
            [](const auto&) {
                std::cerr << "Unimplemented Instruction\n";
            },
        }, instruction);

        cpu.program_counter += props.advance_ip_with(instruction);
        keyboard.update_keyup_events();
    }
};
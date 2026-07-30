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

struct Chip8 {

    using ISA = Chip8ISA<Chip8>;
    using Instruction = ISA::Instruction;

    Display<Chip8Display> display;
    CPU cpu;
    Memory mem;

    ISA instructions;
    Chip8Properties props;

    Chip8() : instructions(*this) {};

    uint8_t& reader(uint8_t r) {
        return cpu.reg(r);
    }

    uint8_t& dereferencer(uint8_t r) {
        return mem[cpu.reg(r)];
    }


    template <typename T>
    bool load_rom(T arr) {
        if (arr.size() > 0xFFF - 0x200 + 1)
            return false;

        std::memcpy(mem.memory.data() + 0x200, arr.data(), arr.size());
        cpu = CPU();
        return true;
    }

    std::expected<void, ISA::InstructionError> run() {
        return instructions.decode(mem.fetch(cpu.program_counter))
            .transform([this](const Instruction& i) { execute(i); });
    }

    void _display(const ISA::Display& instr) {
        uint16_t x = *instr.x_coordinate % Chip8Display::DISPLAY_WIDTH;
        uint16_t y = *instr.y_coordinate % Chip8Display::DISPLAY_HEIGHT;
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

    void execute(Instruction instruction) {
        std::visit(overloads{
            [this](const ISA::Clear&) { 
                display.clear_with(Chip8PixelState::OFF); 
            },
            [this](const ISA::Jump& i) { 
                cpu.program_counter = i.imm.v; 
            },
            [this](const ISA::CallSubroutine& i) {
                cpu.function_pointers.push(cpu.program_counter);
                cpu.program_counter = i.imm.v;
            },
            [this](const ISA::ReturnSubroutine&) {
                const auto top = cpu.function_pointers.top();
                cpu.function_pointers.pop(); 
                cpu.program_counter = top;
            },
            [this](const ISA::SkipEqImm& i) {
                cpu.program_counter += props.skip(
                    i.v1.v() == i.v2.v
                );
            },
            [this](const ISA::SkipNeqImm& i) {
                cpu.program_counter += props.skip(
                    i.v1.v() != i.v2.v
                );
            },
            [this](const ISA::SkipEqReg& i) {
                cpu.program_counter += props.skip(
                    i.v1.v() == i.v2.v()
                );
            },
            [this](const ISA::SkipNeqReg& i) {
                cpu.program_counter += props.skip(
                    i.v1.v() != i.v2.v()
                );
            },
            [this](const ISA::SetImm& i) {
                *i.dst = i.src.v;
            },
            [this](const ISA::SetReg& i) {
                *i.dst = *i.src;
            },
            [this](const ISA::SetIndexReg& i) {
                cpu.index_register = i.imm.v;
            },
            [this](const ISA::Add& i) {
                *i.dst += i.src.v;
            },
            [this](const ISA::Display& i) { _display(i); },
            // [](const auto&) {
            //     std::cerr << "Unimplemented Instruction\n";
            // },
        }, instruction);

        cpu.program_counter += props.advance_ip_with<ISA>(instruction);
    }
};
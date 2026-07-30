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

    using Instruction = Chip8ISA::Instruction;

    Display<Chip8Display> display;
    CPU cpu;
    Memory mem;
    Chip8Properties props;

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

    void _display(const Chip8ISA::Display& instr) {
        uint16_t x = cpu.reg(instr.x_coordinate.r) % Chip8Display::DISPLAY_WIDTH;
        uint16_t y = cpu.reg(instr.y_coordinate.r) % Chip8Display::DISPLAY_HEIGHT;
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
            [this](const Chip8ISA::Clear&) { 
                display.clear_with(Chip8PixelState::OFF); 
            },
            [this](const Chip8ISA::Jump& i) { 
                cpu.program_counter = i.imm.v; 
            },
            [this](const Chip8ISA::CallSubroutine& i) {
                cpu.function_pointers.push(cpu.program_counter);
                cpu.program_counter = i.imm.v;
            },
            [this](const Chip8ISA::ReturnSubroutine&) {
                const auto top = cpu.function_pointers.top();
                cpu.function_pointers.pop(); 
                cpu.program_counter = top;
            },
            [this](const Chip8ISA::SkipEqImm& i) {
                cpu.program_counter += 
                    props.skip(cpu.reg(i.reg.r) == i.imm.v);
            },
            [this](const Chip8ISA::SkipNeqImm& i) {
                cpu.program_counter += 
                    props.skip(cpu.reg(i.reg.r) != i.imm.v);
            },
            [this](const Chip8ISA::SkipEqReg& i) {
                cpu.program_counter += props.skip(
                    cpu.reg(i.r1.r) == cpu.reg(i.r2.r)
                );
            },
            [this](const Chip8ISA::SkipNeqReg& i) {
                cpu.program_counter += props.skip(
                    cpu.reg(i.r1.r) != cpu.reg(i.r2.r)
                );
            },
            [this](const Chip8ISA::SetImm& i) {
                cpu.reg(i.dst.r) = i.src.v;
            },
            [this](const Chip8ISA::SetReg& i) {
                cpu.reg(i.dst.r) = cpu.reg(i.src.r);
            },
            [this](const Chip8ISA::SetIndexReg& i) {
                cpu.index_register = i.imm.v;
            },
            [this](const Chip8ISA::Add& i) {
                cpu.reg(i.dst.r) += i.src.v;
            },
            [this](const Chip8ISA::Display& i) { _display(i); },
            // [](const auto&) {
            //     std::cerr << "Unimplemented Instruction\n";
            // },
        }, instruction);

        cpu.program_counter += props.advance_ip_with(instruction);
    }
};
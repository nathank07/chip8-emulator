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
    Memory memory;
    Chip8Properties props;

    template <typename T>
    bool load_rom(T arr) {
        if (arr.size() > 0xFFF - 0x200 + 1)
            return false;

        std::memcpy(memory.memory.data() + 0x200, arr.data(), arr.size());
        cpu = CPU();
        return true;
    }

    std::expected<void, Chip8ISA::InstructionError> run() {
        return Chip8ISA::decode(memory.fetch(cpu.program_counter))
            .transform([this](const Instruction& i) { execute(i); });
    }

    void _display(const Chip8ISA::Display& i) {
        auto x = cpu.reg(i.x_coordinate.r) & (Chip8Display::DISPLAY_WIDTH  - 1);
        auto y = cpu.reg(i.y_coordinate.r) & (Chip8Display::DISPLAY_HEIGHT - 1);
        
        // Todo: implement sprite loading
        display.draw_batch([this, i, x, y](auto& d){

            bool set_off = false;

            d.with_pixel(x, y, [&set_off](Chip8Display::Color& c) {
                if (c.enabled)
                    set_off = true;
                c.enabled = !c.enabled;
            });

            cpu.reg(0xF) = static_cast<uint8_t>(set_off);

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
            [this](const Chip8ISA::SetImm& i) {
                cpu.reg(i.reg.r) = i.imm.v;
            },
            [this](const Chip8ISA::Add& i) {
                cpu.reg(i.reg.r) += i.imm.v;
            },
            [this](const Chip8ISA::SetIndexReg& i) {
                cpu.index_register = i.imm.v;
            },
            [this](const Chip8ISA::Display& i) { _display(i); },
            [](const auto&) {
                std::cerr << "Unimplemented Instruction\n";
            },
        }, instruction);

        cpu.program_counter += props.advance_ip_with(instruction);
    }
};
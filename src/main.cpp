/*
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include "chip8.hpp"
#include "SDL_vars.hpp"
#include "instructions.hpp"

static Chip8 c8;
static uint64_t prev_ns;
static uint64_t instruction_timer_ns;
static uint64_t ticks_ns;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return SDL_APP_FAILURE;
    }

    std::ifstream rom(argv[1], std::ios::binary);
    if (!rom) {
        std::cerr << "Couldn't open ROM file: " << argv[1] << std::endl;
        return SDL_APP_FAILURE;
    }

    std::vector<uint8_t> rom_data(
        (std::istreambuf_iterator<char>(rom)),
        std::istreambuf_iterator<char>()
    );

    if (!c8.load_rom(rom_data)) {
        std::cerr << "ROM too large to fit in memory: " << argv[1] << std::endl;
        return SDL_APP_FAILURE;
    }

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Chip 8", 
        SDL_WIDTH * SDL_SCALE, SDL_HEIGHT * SDL_SCALE, 
        SDL_WINDOW_OPENGL, &window, &renderer)
    ) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderLogicalPresentation(
        renderer, SDL_WIDTH, SDL_HEIGHT,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
    );

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SDL_WIDTH, SDL_HEIGHT
    );
    if (!texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    c8.display.with_palette([](Chip8Palette& p){
        p.background = 0xffc8ddff;
        p.foreground = 0xfb6f92ff;
    });

    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        const auto action = [event](uint8_t key) {
            event->type == SDL_EVENT_KEY_DOWN ?
                c8.keyboard.keydown_event(key) :
                c8.keyboard.keyup_event(key);
        };

        switch (event->key.scancode) {
            case SDL_Scancode::SDL_SCANCODE_1: { action(0x1); break; }
            case SDL_Scancode::SDL_SCANCODE_2: { action(0x2); break; }
            case SDL_Scancode::SDL_SCANCODE_3: { action(0x3); break; }
            case SDL_Scancode::SDL_SCANCODE_4: { action(0xC); break; }
            case SDL_Scancode::SDL_SCANCODE_Q: { action(0x4); break; }
            case SDL_Scancode::SDL_SCANCODE_W: { action(0x5); break; }
            case SDL_Scancode::SDL_SCANCODE_E: { action(0x6); break; }
            case SDL_Scancode::SDL_SCANCODE_R: { action(0xD); break; }
            case SDL_Scancode::SDL_SCANCODE_A: { action(0x7); break; }
            case SDL_Scancode::SDL_SCANCODE_S: { action(0x8); break; }
            case SDL_Scancode::SDL_SCANCODE_D: { action(0x9); break; }
            case SDL_Scancode::SDL_SCANCODE_F: { action(0xE); break; }
            case SDL_Scancode::SDL_SCANCODE_Z: { action(0xA); break; }
            case SDL_Scancode::SDL_SCANCODE_X: { action(0x0); break; }
            case SDL_Scancode::SDL_SCANCODE_C: { action(0xB); break; }
            case SDL_Scancode::SDL_SCANCODE_V: { action(0xF); break; }
            default: break;
        }
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const uint64_t instr_ps = c8.props.instructions_per_second_ns();
    const uint64_t ticks_ps = c8.props.ticks_per_second_ns();

    uint64_t current_ns = SDL_GetTicksNS();

    if (prev_ns == 0)
        prev_ns = current_ns;

    uint64_t elapsed_ns = current_ns - prev_ns;
    prev_ns = current_ns;
    
    instruction_timer_ns += elapsed_ns;
    ticks_ns += elapsed_ns;

    while (instruction_timer_ns >= instr_ps) {
        auto ok = c8.run();
        if (!ok) {
            SDL_Log("Program crashed because an unimplemented instruction: %04X\n", ok.error().bytes);
            SDL_Delay(5000);
            return SDL_APP_FAILURE;
        }
        instruction_timer_ns -= instr_ps;
    }

    while (ticks_ns >= ticks_ps) {
        c8.cpu.sound_timer = std::max(0, c8.cpu.sound_timer - 1);    
        c8.cpu.delay_timer = std::max(0, c8.cpu.delay_timer - 1);
        ticks_ns -= ticks_ps;
    }
    
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(1);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
}
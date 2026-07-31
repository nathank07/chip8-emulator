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
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    c8.run();
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
}
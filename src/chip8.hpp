#include "display.hpp"
#include "memory.hpp"
#include "SDL_vars.hpp"


struct Chip8 {
    Display<Color, decltype(draw)>& display;
    Memory& memory;
};
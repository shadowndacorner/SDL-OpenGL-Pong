#pragma once
#include <stdint.h>
#include <SDL_scancode.h>

struct input_context
{
    uint8_t keyboard_state[SDL_NUM_SCANCODES];
};

void inputFrameBegin(input_context* ctxt);
void setKeyPressed(input_context* ctxt, SDL_Scancode code);
void setKeyReleased(input_context* ctxt, SDL_Scancode code);

bool isKeyHeld(input_context* ctxt, SDL_Scancode code);
bool wasKeyPressed(input_context* ctxt, SDL_Scancode code);
bool wasKeyReleased(input_context* ctxt, SDL_Scancode code);
#include <input.hpp>
#define PRESSED_FLAG 0b01
#define CHANGED_FLAG 0b10

bool isKeyHeld(input_context* ctxt, SDL_Scancode code)
{
    auto& ks = ctxt->keyboard_state;
    return ks[code] & PRESSED_FLAG;
}

bool wasKeyPressed(input_context* ctxt, SDL_Scancode code)
{
    auto& ks = ctxt->keyboard_state;
    return ks[code] == 0b11;
}

bool wasKeyReleased(input_context* ctxt, SDL_Scancode code)
{
    auto& ks = ctxt->keyboard_state;
    return ks[code] == 0b10;
}

void inputFrameBegin(input_context* ctxt)
{
    auto& ks = ctxt->keyboard_state;
    for(size_t i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        ks[i] = ks[i] & PRESSED_FLAG;
    }
}

void setKeyPressed(input_context* ctxt, SDL_Scancode code)
{
    auto& ks = ctxt->keyboard_state;
    ks[code] = 0b11;
}

void setKeyReleased(input_context* ctxt, SDL_Scancode code)
{
    auto& ks = ctxt->keyboard_state;
    ks[code] = 0b10;
}
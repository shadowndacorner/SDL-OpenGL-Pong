#define SDL_MAIN_HANDLED
// GL_CHECK find and replace regex
// gl[A-Z]([A-Za-z]+)\([A-Za-z_0-9, *+&->]+\)

#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include <input.hpp>
#include <game.hpp>
#include <render.hpp>
#include <window.hpp>

bool mainLoop(SDL_Window* window, input_context* input, game_state* state, window_state* windowState, render_context* renderCtx)
{
    auto ticks = SDL_GetTicks();
    state->delta_ticks = ticks - state->ticks;
    state->deltaTime = double(state->delta_ticks) / 1000.0;
    state->ticks = ticks;

    inputFrameBegin(input);

    SDL_Event ev;
    while(SDL_PollEvent(&ev))
    {
        switch(ev.type)
        {
            case SDL_QUIT:
            {
                return false;
            }
            case SDL_WINDOWEVENT_RESIZED:
            {
                windowState->width = ev.window.data1;
                windowState->height = ev.window.data2;
                break;
            }
            case SDL_KEYDOWN:
            {
                setKeyPressed(input, ev.key.keysym.scancode);
                break;
            }
            case SDL_KEYUP:
            {
                setKeyReleased(input, ev.key.keysym.scancode);
                break;
            }
        }
    }

    const double fixedDelta = 1.0 / 60.0;
    while(state->fixedTime < double(state->ticks) / 1000.)
    {
        fixed_update(fixedDelta, input, state);
        state->fixedTime += fixedDelta;
    }
    update(input, state);
    render(window, state, windowState, renderCtx);

    SDL_GL_SwapWindow(window);
    return true;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    
    auto window = SDL_CreateWindow("programming horror pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    
    // Init SDL GL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // turns on vsync
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    auto ctx = SDL_GL_CreateContext(window);
    if (!ctx) return -1;

    SDL_GL_MakeCurrent(window, ctx);

    int fail = gladLoadGLLoader(SDL_GL_GetProcAddress);
    if (fail == 0)
    {
        fprintf(stderr, "Failed to initialize OpenGL\n");
        return -2;
    }
    
    input_context input;
    window_state windowState;
    render_context render;
    game_state gameState;


    // Zero out everything
    memset(&gameState, 0, sizeof(gameState));
    memset(&input, 0, sizeof(input));
    memset(&windowState, 0, sizeof(windowState));
    memset(&render, 0, sizeof(render));

    reset_game_state(&gameState);

    SDL_GetWindowSize(window, &windowState.width, &windowState.height);

    if (!render_init(&render))
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -3;
    }

    while(mainLoop(window, &input, &gameState, &windowState, &render)){}
    return 0;
}
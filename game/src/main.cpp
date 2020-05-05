#define SDL_MAIN_HANDLED
// GL_CHECK find and replace regex
// gl[A-Z]([A-Za-z]+)\([A-Za-z_0-9, *+&->]+\)

#include <glm/glm.hpp>
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include <glad/glad.h>
#include <gl_util.h>

#define MAX_INSTANCES 64
#define MAX_ENTITIES 0xff
struct render_instance_data
{
    glm::mat4 model;
};

struct camera_ubo_data
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct render_context
{
    GLuint shaderProgram;
    GLuint quadVbo;
    GLuint quadIbo;
    
    GLuint cameraUbo;
    GLuint instanceUbo;

    render_instance_data instanceData[MAX_INSTANCES];
    camera_ubo_data cameraData[MAX_INSTANCES];
};

struct vertex_data
{
    glm::vec4 position;
};

void render_init(render_context* ctx)
{
    GL_CHECK(glGenBuffers(1, &ctx->quadVbo));
    GL_CHECK(glGenBuffers(1, &ctx->quadIbo));
    GL_CHECK(glGenBuffers(1, &ctx->cameraUbo));
    GL_CHECK(glGenBuffers(1, &ctx->instanceUbo));
    
    // Init quad VBO
    {
        vertex_data quadBuffer[] = {
            { {0, 0, 0, 1} },
            { {1, 0, 0, 1} },
            { {1, 1, 0, 1} },
            { {0, 1, 0, 1} }
        };

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ctx->quadVbo));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(quadBuffer), quadBuffer, GL_STATIC_DRAW));
    }

    // Init quad IBO
    {
        uint16_t elements[] = {
            0, 1, 2,
            0, 2, 3
        };

        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->quadIbo));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW));
    }

    // Init camera UBO
    {
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ctx->cameraUbo));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, sizeof(ctx->cameraData), 0, GL_DYNAMIC_DRAW));
    }

    // Init instance UBO
    {
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ctx->instanceUbo));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, sizeof(ctx->instanceData), 0, GL_STREAM_DRAW));
    }

    // TODO: Shader program
}

typedef uint8_t entity;
struct entity_data
{
    glm::vec2 pos;
    glm::vec2 size;
    float rotation;
    bool resident;
};

struct game_context
{
    double fixedTime;
    // objects
    entity_data entities[MAX_ENTITIES];
};

struct window_state
{
    double deltaTime;
    uint32_t delta_ticks;
    uint32_t ticks;

    int width, height;
    uint8_t keyboard[SDL_NUM_SCANCODES];
};

void fixed_update(double dt, game_context* state)
{

}

void render(SDL_Window* window, game_context* state, window_state* windowState, render_context* renderCtx)
{
    GL_CHECK(glClearColor(1, 0, 0, 1));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

    // TODO: Set up camera data
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, renderCtx->instanceUbo));
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(renderCtx->cameraData), &renderCtx->cameraData));

    int numInstances = 0;
    // Note this isn't great, but quick to implement
    for(int i = 0; i < MAX_ENTITIES; ++i)
    {
        auto& ent = state->entities[i];
        if (!ent.resident)
            continue;

        render_instance_data& dat = renderCtx->instanceData[numInstances++];
        // TODO: Init model matrix
        if (numInstances == MAX_INSTANCES)
        {
            // Upload instance data
            GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(render_instance_data) * numInstances, &renderCtx->instanceData));
            
            // 6 corresponds to number of elements to draw
            GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, numInstances));
            numInstances = 0;
        }
    }

    if (numInstances > 0)
    {
        // Same as above, upload instance data
        GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(render_instance_data) * numInstances, &renderCtx->instanceData));
        
        // then draw the instances
        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, numInstances));
    }
}

bool mainLoop(SDL_Window* window, game_context* state, window_state* windowState, render_context* renderCtx)
{
    auto ticks = SDL_GetTicks();
    windowState->delta_ticks = ticks - windowState->ticks;
    windowState->deltaTime = double(windowState->delta_ticks) / 1000.0;
    windowState->ticks = ticks - ticks;

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
        }
    }

    const double fixedDelta = 1.0 / 60.0;
    while(state->fixedTime < double(windowState->ticks) / 1000.)
    {
        fixed_update(fixedDelta, state);
        state->fixedTime += fixedDelta;
    }
    //render(window, state, windowState, renderCtx);

    SDL_GL_SwapWindow(window);
    return true;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    
    auto window = SDL_CreateWindow("programming horror pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
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
        // fatal error here, failed to initialize opengl
        return -2;
    }
    
    window_state windowState;
    render_context render;
    game_context start;

    // Zero out everything
    memset(&windowState, 0, sizeof(windowState));
    memset(&render, 0, sizeof(render));
    memset(&start, 0, sizeof(start));

    SDL_GetWindowSize(window, &windowState.width, &windowState.height);

    render_init(&render);

    while(mainLoop(window, &start, &windowState, &render)){}

    return 0;
}
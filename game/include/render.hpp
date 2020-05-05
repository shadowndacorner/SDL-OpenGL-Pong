#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <gl_util.h>
#include <game.hpp>
#include <window.hpp>

#define MAX_INSTANCES 64
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
    GLuint quadVao;
    GLuint quadVbo;
    GLuint quadIbo;
    
    GLuint cameraUbo;
    GLuint instanceUbo;

    camera_ubo_data cameraData;
    render_instance_data instanceData[MAX_INSTANCES];
};

struct vertex_data
{
    glm::vec4 position;
};

int render_init(render_context* ctx);

struct SDL_Window;
void render(SDL_Window* window, game_state* state, window_state* windowState, render_context* renderCtx);
#include <vector>
#include <SDL.h>
#include <render.hpp>
#include <glm/gtc/matrix_transform.hpp>

const char vertSource[] = R"(
#version 330 core
layout (location = 0) in vec4 pos;

layout(std140) uniform CameraData
{
	mat4 view_matrix;
	mat4 projection_matrix;
};

struct instance_data
{
    mat4 model_matrix;
};

layout(std140) uniform ObjectConstants
{
	instance_data instances[64];
};

void main()
{
    gl_Position = projection_matrix * view_matrix * instances[gl_InstanceID].model_matrix * pos;
}
)";

const char fragSource[] = R"(
#version 330 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1, 1, 1, 1);
}
)";

inline GLuint createShader(GLenum type, const char* src, size_t size)
{
    // Adapted from https://www.khronos.org/opengl/wiki/Shader_Compilation#Shader_object_compilation
    GL_CHECK(GLuint shader = glCreateShader(type));

    // Get strings for glShaderSource.
    GL_CHECK(glShaderSource(shader, 1, &src, 0));
    GL_CHECK(glCompileShader(shader));

    GLint isCompiled = 0;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));

    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        GL_CHECK(glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]));

        switch (type)
        {
        case GL_VERTEX_SHADER:
            fprintf(stderr, "Failed to compile vertex shader: ");
            break;
        case GL_FRAGMENT_SHADER:
            fprintf(stderr, "Failed to compile fragment shader: ");
            break;
        default:
            fprintf(stderr, "Failed to compile unknown shader type: ");
            break;
        }

        fprintf(stderr, "%s\n", errorLog.data());

        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        GL_CHECK(glDeleteShader(shader)); // Don't leak the shader.
        return -1;
    }

    // Shader compilation is successful.
    return shader;
}

inline GLuint createProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    GL_CHECK(GLuint program = glCreateProgram());

    // Attach our shaders to our program
    GL_CHECK(glAttachShader(program, vertexShader));
    GL_CHECK(glAttachShader(program, fragmentShader));

    // Link our program
    GL_CHECK(glLinkProgram(program));

    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked));
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength));

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        GL_CHECK(glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]));
        
        // We don't need the program anymore.
        GL_CHECK(glDeleteProgram(program));

        // In this simple program, we'll just leave
        return ~0;
    }

    // Always detach shaders after a successful link.
    GL_CHECK(glDetachShader(program, vertexShader));
    GL_CHECK(glDetachShader(program, fragmentShader));
    return program;
}

int render_init(render_context* ctx)
{
    GL_CHECK(glGenBuffers(1, &ctx->quadVbo));
    GL_CHECK(glGenBuffers(1, &ctx->quadIbo));
    GL_CHECK(glGenBuffers(1, &ctx->cameraUbo));
    GL_CHECK(glGenBuffers(1, &ctx->instanceUbo));
    
    // Init quad VBO
    {
        // quads are [-1,1] so we can just scale for bounds
        vertex_data quadBuffer[] = {
            { {-1, -1, 0, 1} },
            { {1, -1, 0, 1} },
            { {1, 1, 0, 1} },
            { {-1, 1, 0, 1} }
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
    
    // Init quad VAO
    {
        GL_CHECK(glGenVertexArrays(1, &ctx->quadVao));
        GL_CHECK(glBindVertexArray(ctx->quadVao));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0));
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
    {
        GLuint vertShader = createShader(GL_VERTEX_SHADER, vertSource, sizeof(vertSource));
        GLuint fragShader = createShader(GL_FRAGMENT_SHADER, fragSource, sizeof(fragSource));
        if (vertShader == GLuint(-1) || fragShader == GLuint(-1))
        {
            fprintf(stderr, "Failed to initialize shaders\n");
            return 0;
        }
        
        auto program = ctx->shaderProgram = createProgram(vertShader, fragShader);

        if (program == GLuint(-1))
        {
            // Don't leak shaders either.
            GL_CHECK(glDeleteShader(vertShader));
            GL_CHECK(glDeleteShader(fragShader));
            fprintf(stderr, "Failed to initialize shader program\n");
            return 0;
        }

        auto camUniformIdx = GL_CHECK(glGetUniformBlockIndex(program, "CameraData"));
        auto objUniformIdx = GL_CHECK(glGetUniformBlockIndex(program, "ObjectConstants"));
        GL_CHECK(glUniformBlockBinding(program, camUniformIdx, 0));
        GL_CHECK(glUniformBlockBinding(program, objUniformIdx, 1));
    }
    return 1;
}

void render(SDL_Window* window, game_state* state, window_state* windowState, render_context* renderCtx)
{
    GL_CHECK(glViewport(0, 0, windowState->width, windowState->height));
    //GL_CHECK(glClearColor(sinf((float(state->ticks) / 1000.f) * 2.f) / 2.f +0.5f, 0, 0, 1));
    GL_CHECK(glClearColor(0, 0, 0, 1));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

    auto& playSpace = state->playSpace;
    int isWidth = 0;
    auto max = (playSpace.max.x > playSpace.max.y ? (isWidth = 1) * playSpace.max.x : playSpace.max.y) * 1.2f;

    float width, height;
    if (isWidth)
    {
        width = max;
        height = max / windowState->width * windowState->height;
    }
    else
    {
        width = max * windowState->width / windowState->height;
        height = max;
    }
    

    renderCtx->cameraData.proj = glm::ortho<float>(-width, width, -height, height);
    renderCtx->cameraData.view = glm::identity<glm::mat4>();

    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, renderCtx->cameraUbo));
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(renderCtx->cameraData), &renderCtx->cameraData));
    
    GL_CHECK(glUseProgram(renderCtx->shaderProgram));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, renderCtx->cameraUbo));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 1, renderCtx->instanceUbo));

    // Since there's only one shader and it has predictable instance data, we can keep this bound for
    // the remainder of the rendering loop.
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, renderCtx->instanceUbo));

    // Bind vertex data
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, renderCtx->quadVbo));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderCtx->quadIbo));
    GL_CHECK(glBindVertexArray(renderCtx->quadVao));

    int numInstances = 0;
    for(int i = 0; i < MAX_ENTITIES; ++i)
    {
        auto& ent = state->entities[i];

        // Note this isn't great, but quick to implement and should be totally fine for 255 entities
        // Ideally we'll store the currently allocated entity count as well
        if (!ent.resident)
            continue;

        render_instance_data* dat = &renderCtx->instanceData[numInstances++];
        glm::mat4 trans = glm::translate<float>(glm::mat4(1.f), glm::vec3(ent.pos, 0));
        glm::mat4 scl = glm::scale<float>(glm::mat4(1.f), glm::vec3(ent.bound.size(), 1));
        dat->model = trans * scl;

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
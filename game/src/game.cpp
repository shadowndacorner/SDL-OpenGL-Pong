#include <game.hpp>

entity createEntity(game_state* game)
{
    auto& ents = game->entities;
    for(entity i = 0; i < MAX_ENTITIES; ++i)
    {
        if (!ents[i].resident)
        {
            ents[i].resident = true;
            return entity(i);
        }
    }
    return -1;
}

void destroyEntity(game_state* game, entity ent)
{
   game->entities[ent].resident = false; 
}

entity_data* getEntityData(game_state* game, entity ent)
{
    return &game->entities[ent];
}

glm::vec2 computePaddleInput(input_context* input, SDL_Scancode up, SDL_Scancode down)
{
    using namespace glm;
    vec2 res = {0, 0};
    if (isKeyHeld(input, up))
    {
        res += vec2(0, 1);
    }
    if (isKeyHeld(input, down))
    {
        res -= vec2(0, 1);
    }
    return res;
}

bool ballInPlaySpace(game_state* state)
{
    auto ballDat = getEntityData(state, state->ball);
    return state->playSpace.intersects({0,0}, ballDat->bound, ballDat->pos);
}

void update(input_context* input, game_state* state)
{
    if (!state->playing && wasKeyPressed(input, SDL_SCANCODE_SPACE))
    {
        state->playing = true;
        state->ball = createEntity(state);
        auto& ball = *getEntityData(state, state->ball);
        ball.velocity = {4, 0};
        
        const float ballSize = 0.05f;
        ball.bound = { {-ballSize, -ballSize}, {ballSize, ballSize} };
    }
}

void fixed_update(double dt, input_context* input, game_state* state)
{
    const float moveSpeed = 8;
    if (state->playing)
    {
        float dtf = float(dt);
        auto leftInput = computePaddleInput(input, SDL_SCANCODE_W, SDL_SCANCODE_S) * dtf;
        auto rightInput = computePaddleInput(input, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN) * dtf;
        auto left = getEntityData(state, state->leftPaddle);
        auto right = getEntityData(state, state->rightPaddle);

        left->pos += leftInput * moveSpeed;
        right->pos += rightInput * moveSpeed;

        auto& ball = *getEntityData(state, state->ball);
        ball.pos += ball.velocity * dtf;

        // TODO: Paddle collision

        if (!ballInPlaySpace(state))
        {
            reset_game_state(state);
            return;
        }
    }
}

void reset_game_state(game_state* state)
{
    state->playing = false;
    memset(&state->entities, 0, sizeof(state->entities));
    auto& left = *getEntityData(state, (state->leftPaddle = createEntity(state)));
    auto& right = *getEntityData(state, (state->rightPaddle = createEntity(state)));

    bounds paddleBounds = {{-0.1, -1}, {0.1, 1}};
    left.bound = paddleBounds;
    right.bound = paddleBounds;
    left.pos = {-6, 0};
    right.pos = {6, 0};
    
    auto& bounds = state->playSpace;
    bounds.min = { -8, -10 };
    bounds.max = { 8, 10 };
}
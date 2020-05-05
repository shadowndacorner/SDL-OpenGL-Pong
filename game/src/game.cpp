#include <time.h>
#include <game.hpp>

entity createEntity(game_state* game)
{
    auto& ents = game->entities;
    for(entity i = 0; i < MAX_ENTITIES; ++i)
    {
        if (!ents[i].resident)
        {
            memset(&ents[i], 0, sizeof(entity_data));
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
    return state->playSpace.intersects(ballDat->bound, ballDat->pos);
}

bool ballCollideWithPaddle(game_state* state, entity paddle)
{
    auto& bdata = *getEntityData(state, state->ball);
    auto& pdata = *getEntityData(state, paddle);
    if (glm::dot(bdata.velocity, pdata.normal) < 0 && pdata.bound.intersects(pdata.pos, bdata.bound, bdata.pos))
    {
        auto len = bdata.speed + glm::length(pdata.velocity) + 0.02f;
        auto nrm = glm::normalize(glm::reflect(bdata.velocity, pdata.normal) + pdata.velocity);
        bdata.velocity = nrm;
        bdata.speed = len;
        bdata.additionalSpeed = 12.0f;
        return true;
    }
    return false;
}

inline void spawnBall(game_state* state)
{
    state->ball = createEntity(state);
    auto& ball = *getEntityData(state, state->ball);

    srand(uint32_t(time(0)));
    auto rX = float(rand() % 500 + 500) / 1000.f;
    auto rY = float(rand() % 500 + 500) / 1000.f;
    auto sX = rand() % 2 ? -1 : 1;
    auto sY = rand() % 2 ? -1 : 1;

    ball.velocity = glm::normalize(glm::vec2{rX * sX, (rY * sY) / 2.f});
    ball.speed = 8.f;
    
    // debugging wall collisions
    //ball.velocity = glm::normalize(glm::vec2{0, 1}) * 4.f;

    const float ballSize = 0.1f;
    ball.bound = { {-ballSize, -ballSize}, {ballSize, ballSize} };
}

void update(input_context* input, game_state* state)
{
    if (!state->playing)
    {
        if (wasKeyPressed(input, SDL_SCANCODE_SPACE))
        {
            reset_game_state(state);
            state->playing = true;
            spawnBall(state);
        }
    }
    else
    {
        if (wasKeyPressed(input, SDL_SCANCODE_ESCAPE))
        {
            reset_game_state(state);
        }
    }
}

void tryMovePaddle(game_state* state, entity_data* paddle, const glm::vec2& unscaledDelta, float dt)
{
    auto oldPos = paddle->pos;
    paddle->pos += unscaledDelta * dt;

    auto topWall = getEntityData(state, state->topWall);
    auto bottomWall = getEntityData(state, state->bottomWall);

    paddle->velocity = {0,0};
    if (    paddle->bound.intersects(paddle->pos, topWall->bound, topWall->pos)
        ||  paddle->bound.intersects(paddle->pos, bottomWall->bound, bottomWall->pos))
    {
        paddle->pos = oldPos;
        paddle->velocity = unscaledDelta;
    }
}

void fixed_update(double dt, input_context* input, game_state* state)
{
    const float moveSpeed = 8;
    float dtf = float(dt);
    auto leftInput = computePaddleInput(input, SDL_SCANCODE_W, SDL_SCANCODE_S);
    auto rightInput = computePaddleInput(input, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN);
    auto left = getEntityData(state, state->leftPaddle);
    auto right = getEntityData(state, state->rightPaddle);

    tryMovePaddle(state, left, leftInput * moveSpeed, dtf);
    tryMovePaddle(state, right, rightInput * moveSpeed, dtf);

    if (state->playing)
    {
        auto& ball = *getEntityData(state, state->ball);
        auto bDelta = (ball.velocity) * (ball.speed + ball.additionalSpeed) * dtf;
        ball.pos += bDelta;
        ball.additionalSpeed -= dtf * 8.f;
        if (ball.additionalSpeed < 0) ball.additionalSpeed = 0;

        // Collide with paddles
        // If statement bc it should only be allowed to collide with
        // one paddle/wall per tick
        ballCollideWithPaddle(state, state->leftPaddle) || 
        ballCollideWithPaddle(state, state->rightPaddle) || 
        ballCollideWithPaddle(state, state->bottomWall) || 
        ballCollideWithPaddle(state, state->topWall);

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
    
    auto& worldBounds = state->playSpace;
    worldBounds.min = { -16, -10 };
    worldBounds.max = { 16, 10 };

    // Init paddles
    {
        auto& left = *getEntityData(state, (state->leftPaddle = createEntity(state)));
        auto& right = *getEntityData(state, (state->rightPaddle = createEntity(state)));
        
        bounds paddleBounds = {{-0.2, -2}, {0.2, 2}};
        left.bound = paddleBounds;
        left.normal = {1, 0};
        left.pos = {worldBounds.min.x * 0.8f, 0};

        right.bound = paddleBounds;
        right.normal = {-1, 0};
        right.pos = {worldBounds.max.x * 0.8f, 0};
    }
    
    // Init walls
    {
        auto& top = *getEntityData(state, (state->topWall = createEntity(state)));
        auto& bottom = *getEntityData(state, (state->bottomWall = createEntity(state)));

        bounds wallBounds = {{worldBounds.min.x, -0.25}, {worldBounds.max.x, 0.25}};
        bottom.bound = wallBounds;
        bottom.normal = {0, 1};

        top.bound = wallBounds;
        top.normal = {0, -1};

        bottom.pos = {0, worldBounds.min.y};
        top.pos = {0, worldBounds.max.y};
    }
}
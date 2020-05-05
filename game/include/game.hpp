#pragma once
#include <stdint.h>
#include <glm/glm.hpp>
#include <input.hpp>

#define MAX_ENTITIES 0xff
typedef uint8_t entity;
struct bounds
{
    glm::vec2 min;
    glm::vec2 max;

    inline bool intersects(const bounds& rhs)
    {
        auto& rmin = rhs.min;
        auto& rmax = rhs.max;

        bool xIntersect = min.x < rmax.x && max.x > rmin.x;
        bool yIntersect = min.y < rmax.y && max.y > rmax.y;
        return xIntersect && yIntersect;
    }

    inline bool intersects(const glm::vec2& mPos, const bounds& rhs, const glm::vec2& rPos)
    {
        // TODO: This can be optimized, but meh
        bounds mb = { min + mPos, max + mPos };
        bounds rb = { rhs.min + rPos, rhs.max + rPos };
        return mb.intersects(rb);
    }

    inline bool intersects(const bounds& rhs, const glm::vec2& rPos)
    {
        // TODO: This can be optimized, but meh
        bounds rb = { rhs.min + rPos, rhs.max + rPos };
        return intersects(rb);
    }

    inline glm::vec2 size()
    {
        return (max - min) / 2.f;
    }
};

struct entity_data
{
    bounds bound;
    glm::vec2 pos;
    glm::vec2 velocity;
    glm::vec2 normal;

    float speed;
    float additionalSpeed;
    bool resident;
};

struct game_state
{
    double fixedTime;
    double deltaTime;
    uint32_t delta_ticks;
    uint32_t ticks;

    // game state
    bool playing;
    entity_data entities[MAX_ENTITIES];

    // entities
    entity leftPaddle;
    entity rightPaddle;
    
    entity topWall;
    entity bottomWall;

    entity ball;
    bounds playSpace;
};

void reset_game_state(game_state* state);
entity createEntity(game_state* game);
void destroyEntity(game_state* game, entity ent);
entity_data* getEntityData(game_state* game, entity ent);

void update(input_context* input, game_state* state);
void fixed_update(double dt, input_context* input, game_state* state);
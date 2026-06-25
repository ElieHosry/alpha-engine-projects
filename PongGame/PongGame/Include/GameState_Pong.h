#ifndef PONG_GAMESTATE_PONG_H_
#define PONG_GAMESTATE_PONG_H_

#include "AEEngine.h"

struct AABB
{
    float minX, minY, maxX, maxY;
};

struct Paddle
{
    AEVec2 pos;      // center in world space
    float  halfW;
    float  halfH;

    AABB GetAABB() const {
        return { pos.x - halfW, pos.y - halfH,
                 pos.x + halfW, pos.y + halfH };
    }
};

struct Ball
{
    AEVec2 pos;
    AEVec2 vel;
    float  halfSize;

    AABB GetAABB() const {
        return { pos.x - halfSize, pos.y - halfSize,
                 pos.x + halfSize, pos.y + halfSize };
    }
};

inline bool AABBOverlap(const AABB& a, const AABB& b)
{
    return a.maxX > b.minX && a.minX < b.maxX
        && a.maxY > b.minY && a.minY < b.maxY;
}

enum PongWinner { PONG_WINNER_NONE = 0, PONG_WINNER_PLAYER, PONG_WINNER_NPC };
PongWinner PongGetWinner();

void GameState_PongLoad();
void GameState_PongInit();
void GameState_PongUpdate();
void GameState_PongDraw();
void GameState_PongFree();
void GameState_PongUnload();

#endif // PONG_GAMESTATE_PONG_H_

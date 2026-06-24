#ifndef PONG_GAMESTATEMGR_H_
#define PONG_GAMESTATEMGR_H_

#include "AEEngine.h"
#include "GameStateList.h"

// Current / next / previous state IDs (written by game logic to request transitions)
extern unsigned int gGameStateInit;
extern unsigned int gGameStateCurr;
extern unsigned int gGameStateNext;
extern unsigned int gGameStatePrev;

// Per-state callbacks — bound by GameStateMgrUpdate() each state transition
extern void (*GameStateLoad)();
extern void (*GameStateInit)();
extern void (*GameStateUpdate)();
extern void (*GameStateDraw)();
extern void (*GameStateFree)();
extern void (*GameStateUnload)();

void GameStateMgrInit(unsigned int gameStateInit);
void GameStateMgrUpdate();

#endif // PONG_GAMESTATEMGR_H_

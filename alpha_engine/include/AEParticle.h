#pragma once
#include "AEExport.h"

AE_API void AEParticleInit(int maxParticles);
AE_API void AEParticleFree();
AE_API void AEParticleUpdate(float dt);
AE_API void AEParticleDraw();
AE_API void AEParticleClear();
AE_API void AEParticleEmit(float x, float y,
                            float vxMin, float vxMax,
                            float vyMin, float vyMax,
                            float r, float g, float b,
                            float halfSize, float maxLife, int count);

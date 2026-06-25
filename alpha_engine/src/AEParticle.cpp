#include "AEParticle.h"
#include "AEEngine.h"
#include <cstdlib>

struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float halfSize;
    float life;
    float maxLife;
    bool  alive;
};

static Particle*        sPool = nullptr;
static int              sMax  = 0;
static AEGfxVertexList* sMesh = nullptr;

static float randRange(float lo, float hi)
{
    return lo + (hi - lo) * (rand() / (float)RAND_MAX);
}

static AEGfxVertexList* CreateParticleMesh()
{
    AEGfxMeshStart();
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
         0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(
        -0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
         0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
         0.5f,  0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
    return AEGfxMeshEnd();
}

void AEParticleInit(int maxParticles)
{
    sMax  = maxParticles;
    sPool = new Particle[sMax];
    AEParticleClear();
    sMesh = CreateParticleMesh();
}

void AEParticleFree()
{
    if (sMesh) { AEGfxMeshFree(sMesh); sMesh = nullptr; }
    delete[] sPool;
    sPool = nullptr;
    sMax  = 0;
}

void AEParticleClear()
{
    for (int i = 0; i < sMax; ++i)
        sPool[i].alive = false;
}

void AEParticleUpdate(float dt)
{
    for (int i = 0; i < sMax; ++i)
    {
        if (!sPool[i].alive) continue;
        sPool[i].life -= dt;
        if (sPool[i].life <= 0.0f) { sPool[i].alive = false; continue; }
        sPool[i].x += sPool[i].vx * dt;
        sPool[i].y += sPool[i].vy * dt;
    }
}

void AEParticleDraw()
{
    if (!sPool || !sMesh) return;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxTextureSet(NULL, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < sMax; ++i)
    {
        if (!sPool[i].alive) continue;
        float alpha = sPool[i].life / sPool[i].maxLife;
        AEGfxSetColorToMultiply(sPool[i].r, sPool[i].g, sPool[i].b, 1.0f);
        AEGfxSetTransparency(alpha);

        AEMtx33 scale, trans, xform;
        AEMtx33Scale(&scale, sPool[i].halfSize * 2.0f, sPool[i].halfSize * 2.0f);
        AEMtx33Trans(&trans, sPool[i].x, sPool[i].y);
        AEMtx33Concat(&xform, &trans, &scale);
        AEGfxSetTransform(xform.m);
        AEGfxMeshDraw(sMesh, AE_GFX_MDM_TRIANGLES);
    }
}

void AEParticleEmit(float x, float y,
                    float vxMin, float vxMax,
                    float vyMin, float vyMax,
                    float r, float g, float b,
                    float halfSize, float maxLife, int count)
{
    int spawned = 0;
    for (int i = 0; i < sMax && spawned < count; ++i)
    {
        if (sPool[i].alive) continue;
        sPool[i].x        = x;
        sPool[i].y        = y;
        sPool[i].vx       = randRange(vxMin, vxMax);
        sPool[i].vy       = randRange(vyMin, vyMax);
        sPool[i].r        = r;
        sPool[i].g        = g;
        sPool[i].b        = b;
        sPool[i].halfSize = halfSize;
        sPool[i].maxLife  = maxLife;
        sPool[i].life     = maxLife;
        sPool[i].alive    = true;
        ++spawned;
    }
}

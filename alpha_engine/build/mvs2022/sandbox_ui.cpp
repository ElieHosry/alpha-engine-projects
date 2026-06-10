// Game of life assignment solution
#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"

#define GOL_GRID_COLS (1600/20)
#define GOL_GRID_ROWS (900/20)
#define GOL_GRID_BUFFERS 2

#define GOL_ALIVE 1
#define GOL_DEAD 0

#define EFFECT_SIZE 400.f
#define EFFECT_SPEED 20.f
#define PLAYER_SIZE 60.f
#define PLAYER_SPEED 500.f

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

#define TRUE 1
#define FALSE 0

static AEGfxVertexList* square_mesh = nullptr;

static f32 abs(f32 x)
{
    return x > 0 ? x : -x;
}

static f32 lerp(f32 s, f32 e, f32 v)
{
    return s + ((e - s) * v);
}

static bool aabb_to_aabb_collision(
    f32 lhs_x, f32 lhs_y, f32 lhs_hw, f32 lhs_hh,
    f32 rhs_x, f32 rhs_y, f32 rhs_hw, f32 rhs_hh)
{
    if (abs(lhs_x - rhs_x) < (lhs_hw + rhs_hw))
    {
        if (abs(lhs_y - rhs_y) < lhs_hh + rhs_hh)
        {
            return true;
        }
    }
    return false;
}

static void draw_rectangle(f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a, f32 anchor_x = 0.5f, f32 anchor_y = 0.5f)
{

    // If anchor is (0.5, 0.5) which represents the center, map to (0, 0)
    // If anchor is (1.0, 1.0) which represents the top right, map to (-0.5, -0.5)
    // If anchor is (0.0, 0.0) which represents the bottom left, map to (0.5, 0.5)
    f32 ox = lerp(0.5f, -0.5f, anchor_x);
    f32 oy = lerp(0.5f, -0.5f, anchor_y);

    AEGfxSetTransparency(a);
    AEGfxSetColorToAdd(r, g, b, 0.0f);

    AEMtx33 transform;
    AEMtx33Identity(&transform);

    AEMtx33 offset;
    AEMtx33Trans(&offset, ox, oy);

    AEMtx33 scale;
    AEMtx33Scale(&scale, w, h);

    AEMtx33 translate;
    AEMtx33Trans(&translate, x, y);

    AEMtx33Concat(&transform, &scale, &offset);
    AEMtx33Concat(&transform, &translate, &transform);
    AEGfxSetTransform(transform.m);

    AEGfxMeshDraw(square_mesh, AE_GFX_MDM_TRIANGLES);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR    lpCmdLine,
        _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    bool gGameRunning = true;

    // Using custom window procedure
    AESysInit(hInstance, nCmdShow, WINDOW_WIDTH, WINDOW_HEIGHT, 1, 60, true, NULL);

    // Changing the window title
    AESysSetWindowTitle("UI Demo!");

    // Informing the library that we're about to start adding triangles
    AEGfxMeshStart();

    // This shape has 2 triangles that makes up a square
    // Color parameters represent colours as ARGB
    // UV coordinates to read from loaded textures
    AEGfxTriAdd(
            -0.5f, -0.5f, 0xFF000000, 0.0f, 1.0f,  // bottom-left: red
            0.5f, -0.5f, 0xFF000000, 1.0f, 1.0f,   // bottom-right: green
            -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);  // top-left: blue

    AEGfxTriAdd(
            0.5f, -0.5f, 0xFF000000, 1.0f, 1.0f,   // bottom-right: green
            0.5f, 0.5f, 0xFF000000, 1.0f, 0.0f,    // top-right: white
            -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);  // top-left: blue

    // Saving the mesh (list of triangles) in square_mesh
    square_mesh = AEGfxMeshEnd();

    f32 player_hp = 100;
    AEVec2 player_pos;
    AEVec2Set(&player_pos, 0, -50);

    AEVec2 damage_tower_pos;
    AEVec2Set(&damage_tower_pos, WINDOW_WIDTH/4, -50);

    AEVec2 healing_tower_pos;
    AEVec2Set(&healing_tower_pos, -WINDOW_WIDTH/4, -50);

    // Game Loop
    while (gGameRunning)
    {
        f32 dt = (f32)AEFrameRateControllerGetFrameTime();
        // Player input
        {
            AEVec2 dir;
            AEVec2Set(&dir, 0, 0);

            if (AEInputCheckCurr(AEVK_W)) {
                dir.y += 1.f;
            }
            if (AEInputCheckCurr(AEVK_S)) {
                dir.y -= 1.f;
            }
            if (AEInputCheckCurr(AEVK_A)) {
                dir.x -= 1.f;
            }
            if (AEInputCheckCurr(AEVK_D)) {
                dir.x += 1.f;
            }
            if (AEVec2SquareLength(&dir) > 0) {
                AEVec2Normalize(&dir, &dir);
                AEVec2Scale(&dir, &dir, PLAYER_SPEED * dt);
                AEVec2Add(&player_pos, &player_pos, &dir);
            }
        }
        
        // Collision
        if (aabb_to_aabb_collision(
            player_pos.x, player_pos.y, PLAYER_SIZE / 2, PLAYER_SIZE / 2,
            damage_tower_pos.x, damage_tower_pos.y, EFFECT_SIZE / 2, EFFECT_SIZE / 2))
        {
            player_hp -= dt * EFFECT_SPEED;
            if (player_hp <= 0.f)
                player_hp = 0.f;
        }

        if (aabb_to_aabb_collision(
            player_pos.x, player_pos.y, PLAYER_SIZE / 2, PLAYER_SIZE / 2,
            healing_tower_pos.x, healing_tower_pos.y, EFFECT_SIZE / 2, EFFECT_SIZE / 2))
        {
            player_hp += dt * EFFECT_SPEED;
            if (player_hp >= 100.f)
                player_hp = 100.f;
        }


        AESysFrameStart();

        /* RENDER background */
        AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f);


        // Tell the engine to get ready to draw something with color.
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        // Set the the color to multiply to white, so that the sprite can 
        // display the full range of colors (default is black).
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);


        // Set blend mode to AE_GFX_BM_BLEND
        // This will allow transparency.
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);


        // Draw damage tower
        draw_rectangle(damage_tower_pos.x, damage_tower_pos.y, EFFECT_SIZE, EFFECT_SIZE, 1, 0, 0, 0.5f);


        // Draw healing tower
        draw_rectangle(healing_tower_pos.x, healing_tower_pos.y, EFFECT_SIZE, EFFECT_SIZE, 0, 1, 0, 0.5f);

        // Draw Player
        draw_rectangle(player_pos.x, player_pos.y, PLAYER_SIZE, PLAYER_SIZE, 0, 0, 1, 1);

        // Draw UI bar
        {
            float health_bar_width = 0.8f * WINDOW_WIDTH;
            float health_bar_height = 0.05f * WINDOW_HEIGHT;
            float health_bar_x = (0.1f * WINDOW_WIDTH) - WINDOW_WIDTH / 2;
            float health_bar_y = (0.9f * WINDOW_HEIGHT) - WINDOW_HEIGHT / 2;

            draw_rectangle(
                health_bar_x, health_bar_y, 
                health_bar_width, health_bar_height,
                0.5, 0, 0, 1, 
                0, 1);
            draw_rectangle(
                health_bar_x, health_bar_y,
                health_bar_width * (player_hp /100.f), health_bar_height,
                1, 0, 0, 1,
                0, 1);

            int icons_to_show = (int)((player_hp + 9) / 10); // lol
            float icon_x = health_bar_x;
            const float icon_w = health_bar_width / 19; // 10 icons + 9 gaps
            const float icon_h = 0.05f * WINDOW_HEIGHT;
            const float icon_gap = icon_w;

            const float icon_y = 0.8f * WINDOW_HEIGHT - WINDOW_HEIGHT / 2;
            for (int i = 0; i < icons_to_show; ++i) {
                draw_rectangle(icon_x, icon_y, icon_w, icon_h, 1, 0, 0, 1, 0, 1);
                icon_x += icon_w + icon_gap;
            }
        }

        AESysFrameEnd();


        // check if forcing the application to quit
        if (0 == AESysDoesWindowExist())
            gGameRunning = 0;


    }
    AEGfxMeshFree(square_mesh);
    AESysExit();
}

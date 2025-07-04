#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <vector>
#include <cmath>

#include "number_types.h"
#include "sprite.h"

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;
const f32 FPS_MAX       = 144.0;

struct Entity {
    int health = 100;
    int damage = 12;
    f32 speed  = 0.01;
    f32 x;
    f32 y;
    u32 idx_anim;
};

struct Game {
    SDL_Window*   window;
    SDL_Renderer* renderer;

    Img bg;
    Img entity_shadow;

    static constexpr u32 sprite_player_frames[] = { 4, 8, 4, 3, 6, 6, 5, 3, 3, 0 };
    Sprite sprite_player = {
        .img                     = {},
        .max_frames_in_row_count = 10,
        .frames_in_each_row      = std::span{sprite_player_frames},
    };

    Entity              player;
    std::vector<Entity> enemies;

    // delta time
    u64 dt;
};

static Game g = {};

static bool init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL err: %s\n", SDL_GetError());
        return false;
    }

    {
        g.window = SDL_CreateWindow(
            "Fists of Fury",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE
        );

        if (g.window == nullptr) {
            SDL_Log("Window could not be created! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        g.renderer = SDL_CreateRenderer(g.window, nullptr);
        if (g.renderer == nullptr) {
            SDL_Log("Renderer could not be created! SDL err: %s\n", SDL_GetError());
            return false;
        }

        bool ok = SDL_SetRenderLogicalPresentation(
            g.renderer,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
        );
        if (!ok) {
            SDL_Log("Could not set logical presentation! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        bool ok = img_load(g.bg, g.renderer, "assets/art/backgrounds/street-background.png");
        if (!ok) {
            SDL_Log("Failed to load bg img! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        bool ok = img_load(g.entity_shadow, g.renderer, "assets/art/characters/shadow.png");
        if (!ok) {
            SDL_Log("Failed to load shadow img! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        bool ok = sprite_load(g.sprite_player, g.renderer, "assets/art/characters/player.png");
        if (!ok) {
            SDL_Log("Failed to load player sprite! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        // player setup
        g.player.x = SCREEN_WIDTH  / 2;
        g.player.y = SCREEN_HEIGHT / 2;
    }

    {
        // enemies setup
    }

    return true;
}

static void draw_background(SDL_Renderer* r) {
    const SDL_FRect dst = {0, 0, g.bg.width, g.bg.height};
    SDL_RenderTexture(r, g.bg.img, NULL, &dst);
}

static void draw_entity(SDL_Renderer* r, Entity e) {
    const SDL_FRect dst = {e.x, e.y, g.entity_shadow.width, g.entity_shadow.height};
    SDL_RenderTexture(r, g.entity_shadow.img, NULL, &dst);
}

static void update_enemy(Entity* e) {
    // TODO: remove this
    e->health = e->health;
}

enum struct Player_Anim : u32 {
    Standing      = 0,
    Running       = 1,
    PunchingLeft  = 2,
    PunchingRight = 3,
    KickingLeft   = 4,
    KickingRight  = 5,
    KickingLow    = 6,
    Glow          = 7,
    Fallover      = 8
};

static void draw_player(SDL_Renderer* r, const Entity& p) {
    u64 ticks = SDL_GetTicks() / 100;
    u64 idx = ticks % g.sprite_player.frames_in_each_row[p.idx_anim];
    bool ok = sprite_draw_at_dst(g.sprite_player, r, p.x, p.y, p.idx_anim, idx);
    if (!ok) {
        SDL_Log("Failed to draw player sprite! SDL err: %s\n", SDL_GetError());
    }
}

static void update_player(Entity* p) {
    p->x = std::fmod(p->x + p->speed * g.dt, (f32)SCREEN_WIDTH);
    p->y = (f32)SCREEN_HEIGHT / 16;
    if (SDL_GetTicks() > 1000) p->idx_anim = (u32)Player_Anim::PunchingLeft;
}

static void update() {
    for (u64 idx = 0; idx < g.enemies.size(); idx++) {
        update_enemy(&g.enemies[idx]);
    }
    update_player(&g.player);
}

static void draw() {
    SDL_RenderClear(g.renderer);

    draw_background(g.renderer);

    for (u64 idx = 0; idx < g.enemies.size(); idx++) {
        draw_entity(g.renderer, g.enemies[idx]);
    }
    draw_player(g.renderer, g.player);

    SDL_RenderPresent(g.renderer);
}

int main() {
    if (!init()) {
        return 1;
    }

    bool quit = false;
    u64  a    = SDL_GetTicks();
    u64  b    = SDL_GetTicks();

    SDL_Event              e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        a    = SDL_GetTicks();
        g.dt = a - b;

        if (g.dt > 1000 / FPS_MAX) {
            b = a;
            update();
            draw();
        }

    }

    return 0;
}

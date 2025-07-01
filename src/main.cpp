#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "number_types.h"
#include <vector>
#include <cmath>

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;
const f32 FPS_MAX       = 144.0;

struct Img {
    SDL_Texture* img;
    f32          width;
    f32          height;
};

struct Entity {
    int health = 100;
    int damage = 20;
    f32 speed  = 1;
    f32 x;
    f32 y;
};

struct Game {
    SDL_Window*   window;
    SDL_Renderer* renderer;

    Img bg;
    Img entity_shadow;

    Entity              player;
    std::vector<Entity> enemies;

    // delta time
    u64 dt;
};

static Game g = {};

// Has to be called after initializing the renderer.
//
// returns null on error
static bool load_img(const char* path, Img& i) {
    i.img = IMG_LoadTexture(g.renderer, path);
    if (i.img == nullptr) {
        SDL_Log("Could not load img! SDL err: %s\n", SDL_GetError());
        return false;
    }

    bool ok = SDL_GetTextureSize(i.img, &i.width, &i.height);
    if (!ok) {
        SDL_Log("Could not load img size! SDL err: %s\n", SDL_GetError());
        return false;
    }

    ok = SDL_SetTextureScaleMode(i.img, SDL_SCALEMODE_NEAREST);
    if (!ok) {
        SDL_Log("Could not set texture scaling to nearest neighbor! SDL err: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

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
        bool ok = load_img("assets/art/backgrounds/street-background.png", g.bg);
        if (!ok) {
            SDL_Log("Failed to load bg img! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        bool ok = load_img("assets/art/characters/shadow.png", g.entity_shadow);
        if (!ok) {
            SDL_Log("Failed to load shadow img! SDL err: %s\n", SDL_GetError());
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

static void update_enemy(Entity* e) {}

static void update_player(Entity* p) {
    p->x = std::fmod(p->x + 0.01 * g.dt, (f32)SCREEN_WIDTH);
}

static void update() {
    for (int idx = 0; idx < g.enemies.size(); idx++) {
        update_enemy(&g.enemies[idx]);
    }
    update_player(&g.player);
}

static void draw() {
    SDL_RenderClear(g.renderer);

    draw_background(g.renderer);

    for (int idx = 0; idx < g.enemies.size(); idx++) {
        draw_entity(g.renderer, g.enemies[idx]);
    }
    draw_entity(g.renderer, g.player);

    SDL_RenderPresent(g.renderer);
}

int main(int argc, char* argv[]) {
    if (!init()) {
        return 1;
    }

    bool quit  = false;
    u64  a     = SDL_GetTicks();
    u64  b     = SDL_GetTicks();

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        a       = SDL_GetTicks();
        g.dt    = a - b;

        if (g.dt > 1000 / FPS_MAX) {
            b = a;
            update();
            draw();
        }
    }

    return 0;
}

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <algorithm>
#include <variant>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <functional>

#include "game.h"

#include "number_types.h"
#include "sprite.h"
#include "level_info.h"
#include "settings.h"
#include "draw.h"

#include "entities/entity.h"
#include "entities/player.h"
#include "entities/enemy.h"

static Game g = {};

internal Entity& get_player() {
    return g.entities[g.idx_player];
}

internal bool init() {
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
        g.curr_level_info = level_data_get_level(Level::Street);
        g.camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        bool ok = img_load(g.bg, g.renderer, g.curr_level_info.bg_path);
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

    // player setup
    {
        Entity player{};
        player.type                  = Entity_Type::Player;
        player.x                     = SCREEN_WIDTH  / 16;
        player.y                     = SCREEN_HEIGHT / 16;
        player.collision_box_offsets = {18, 45, 12, 5};
        player.shadow_offset         = {17, 48, 14, 2};
        player.default_anim          = (u32)Player_Anim::Standing;
        start_animation(player, (u32)Player_Anim::Standing, true);
        g.entities.push_back(player);
    }

    {
        // enemies setup
    }

    return true;
}

internal void update_entity(Entity& e) {
    switch (e.type) {
        case Entity_Type::Player: {
            update_player(e, g);
            break;
        }

        case Entity_Type::Enemy: {
            update_enemy(e);
            break;
        }
    }
}

internal void y_sort_entities(Game& g) {
    if (g.sorted_indices.size() != g.entities.size()) {
        g.sorted_indices.clear();
        g.sorted_indices.reserve(g.entities.size());
        for (u32 idx = 0; idx < g.entities.size(); idx++) {
            g.sorted_indices.push_back(idx);
        }
    }

    auto sort_fn = [g](u32 a, u32 b) { return g.entities[a].y < g.entities[b].y; };
    std::sort(g.sorted_indices.begin(), g.sorted_indices.end(), sort_fn);
}

internal void update(Game& g) {
    for (u64 idx = 0; idx < g.entities.size(); idx++) {
        update_entity(g.entities[idx]);
    }
    update_camera(get_player(), g);
    y_sort_entities(g);

    g.input_prev  = g.input;
    g.input.punch = false;
    g.input.kick  = false;
}

internal void draw_entity(SDL_Renderer* r, Entity e) {
    switch (e.type) {
        case Entity_Type::Player: {
            draw_player(r, e, g);
            break;
        }

        case Entity_Type::Enemy: {
            draw_enemy(r, e, g);
            break;
        }
            break;
        }
    }
}

internal void draw(const Game& g) {
    SDL_RenderClear(g.renderer);

    draw_level(g.renderer, g);

    for (u32 idx_sorted : g.sorted_indices) {
        draw_entity(g.renderer, g.entities[idx_sorted]);
    }

    SDL_RenderPresent(g.renderer);
}

internal void handle_input(const SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
            case SDLK_S: g.input.left  = true; break;
            case SDLK_F: g.input.right = true; break;
            case SDLK_E: g.input.up    = true; break;
            case SDLK_D: g.input.down  = true; break;
            case SDLK_J: g.input.punch = true; break;
            case SDLK_K: g.input.kick  = true; break;
        }
    }

    if (e.type == SDL_EVENT_KEY_UP) {
        switch (e.key.key) {
            case SDLK_S: g.input.left  = false; break;
            case SDLK_F: g.input.right = false; break;
            case SDLK_E: g.input.up    = false; break;
            case SDLK_D: g.input.down  = false; break;
        }
    }
}

int main() {
    if (!init()) {
        return 1;
    }

    bool quit = false;
    u64  a    = SDL_GetTicks();
    u64  b    = SDL_GetTicks();

    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT: {
                    quit = true;
                    break;
                }

                case SDL_EVENT_KEY_DOWN: {
                    handle_input(e);
                    break;
                }

                case SDL_EVENT_KEY_UP: {
                    handle_input(e);
                    break;
                }
            }
        }

        a    = SDL_GetTicks();
        g.dt = a - b;

        if (g.dt > 1000 / FPS_MAX) {
            b = a;
            update(g);
            draw(g);
        }
    }

    return 0;
}

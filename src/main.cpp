#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>

#include "entities/entity.h"
#include "game.h"

#include "number_types.h"
#include "settings.h"
#include "draw.h"

#include "entities/player.h"
#include "entities/enemy.h"
#include "entities/barrel.h"

static Game g = {};

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
        Entity player = player_init(&g.sprite_player);
        g.entities.push_back(player);
        g.idx_player = g.entities.size() - 1;
    }

    {
        bool ok = sprite_load(g.sprite_barrel, g.renderer, "assets/art/props/barrel.png");
        if (!ok) {
            SDL_Log("Failed to load barrel sprite! SDL err: %s\n", SDL_GetError());
            return false;
        }
    }

    {
        Entity barrel = barrel_init({
            .x = SCREEN_WIDTH / 2,
            .y = 38,
            .health = 20,
            .sprite = &g.sprite_barrel,
        });
        g.entities.push_back(barrel);

        Entity barrel2 = barrel_init({
            .x = SCREEN_WIDTH,
            .y = 38,
            .health = 20,
            .sprite = &g.sprite_barrel,
        });
        g.entities.push_back(barrel2);
    }

    {
        // enemies setup
    }

    return true;
}

internal Update_Result update_entity(Entity& e) {
    Update_Result res;

    switch (e.type) {
        case Entity_Type::Player: {
            res = player_update(e, g);
            break;
        }

        case Entity_Type::Enemy: {
            res = enemy_update(e);
            break;
        }

        case Entity_Type::Barrel: {
            res = barrel_update(e, g);
            break;
        }
    }

    return res;
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
        auto res = update_entity(g.entities[idx]);
        if (res == Update_Result::Remove_Me) g.removal_queue.push_back(idx);
    }

    while (!g.removal_queue.empty()) {
        const auto idx = g.removal_queue.back();
        g.entities.erase(g.entities.begin() + idx);
        g.removal_queue.pop_back();
    }
    y_sort_entities(g);

    g.input_prev  = g.input;
    g.input.punch = false;
    g.input.kick  = false;
}

internal void draw_entity(SDL_Renderer* r, Entity e) {
    switch (e.type) {
        case Entity_Type::Player: {
            player_draw(r, e, g);
            break;
        }

        case Entity_Type::Enemy: {
            enemy_draw(r, e, g);
            break;
        }

        case Entity_Type::Barrel: {
            barrel_draw(r, e, g);
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

struct Key_Binding {
    SDL_Keycode key;
    bool*       input_field;
};

Key_Binding bindings[] = {
    {SDLK_S,     &g.input.left},
    {SDLK_F,     &g.input.right},
    {SDLK_E,     &g.input.up},
    {SDLK_D,     &g.input.down},
    {SDLK_J,     &g.input.punch},
    {SDLK_K,     &g.input.kick},
    {SDLK_SPACE, &g.input.jump},
};

internal void handle_input(const SDL_Event& e) {
    bool pressed = (e.type == SDL_EVENT_KEY_DOWN);
    for (auto& binding : bindings) {
        if (e.key.key == binding.key) {
            *binding.input_field = pressed;
            break;
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

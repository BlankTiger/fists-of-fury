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

enum struct Direction {
    Up,
    Down,
    Left,
    Right
};

struct Entity {
    int       health = 100;
    int       damage = 12;
    f32       speed  = 0.03;
    f32       x;
    f32       y;
    u32       idx_anim;
    Direction dir;

    u32 current_frame      = 0;     // Current frame in the animation
    u64 last_frame_time    = 0;     // When the last frame was shown
    u64 frame_duration_ms  = 100;   // Milliseconds per frame
    bool animation_playing = false;
    bool animation_loop    = true;  // Whether this animation should loop
    u32 default_anim       = 0;     // Animation to return to when current finishes
};

struct Input_State {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool punch = false;
    bool kick  = false;

    bool last_punch_was_left = true;
    bool last_kick_was_left  = true;
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
    Entity player;
    Input_State input;
    Input_State input_prev; // for detecting press -> release

    std::vector<Entity> enemies;

    // delta time
    u64 dt;
};

static Game g = {};

enum struct Player_Anim : u32 {
    Standing       = 0,
    Running        = 1,
    Punching_Left  = 2,
    Punching_Right = 3,
    Kicking_Left   = 4,
    Kicking_Right  = 5,
    Kicking_Low    = 6,
    Got_Hit        = 7,
    Fallover       = 8
};

static void start_animation(Entity* e, u32 anim_idx, bool should_loop = false, u64 frame_time = 100) {
    e->idx_anim          = anim_idx;
    e->current_frame     = 0;
    e->animation_playing = true;
    e->animation_loop    = should_loop;
    e->frame_duration_ms = frame_time;
    e->last_frame_time   = SDL_GetTicks();
}

static bool is_animation_finished(const Entity& e) {
    if (!e.animation_playing) return true;
    if (e.animation_loop) return false;
    return e.current_frame >= g.sprite_player.frames_in_each_row[e.idx_anim] - 1;
}

static void update_animation(Entity* e) {
    if (!e->animation_playing) return;

    u64 current_time = SDL_GetTicks();
    if (current_time - e->last_frame_time >= e->frame_duration_ms) {
        e->current_frame++;
        e->last_frame_time = current_time;

        // Check if animation finished
        if (e->current_frame >= g.sprite_player.frames_in_each_row[e->idx_anim]) {
            if (e->animation_loop) {
                e->current_frame = 0; // Loop back to start
            } else {
                // Animation finished - return to default
                e->animation_playing = false;
                start_animation(e, e->default_anim, true);
            }
        }
    }
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
        g.player.x = SCREEN_WIDTH  / 16;
        g.player.y = SCREEN_HEIGHT / 16;
        g.player.default_anim = (u32)Player_Anim::Standing;
        start_animation(&g.player, (u32)Player_Anim::Standing, true);
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

static void draw_player(SDL_Renderer* r, const Entity& p) {
    SDL_FlipMode flip = (p.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    bool ok = sprite_draw_at_dst(g.sprite_player, r, p.x, p.y, p.idx_anim, p.current_frame, flip);
    if (!ok) {
        SDL_Log("Failed to draw player sprite! SDL err: %s\n", SDL_GetError());
    }
}

static bool input_pressed(bool curr, bool prev) {
    return curr && !prev;
}

static bool input_released(bool curr, bool prev) {
    return !curr && prev;
}

static void update_player(Entity* p) {
    const auto& in      = g.input;
    const auto& in_prev = g.input_prev;

        if (input_pressed(in.punch, in_prev.punch)) {
        if (!p->animation_playing || p->animation_loop) {
            if (g.input.last_punch_was_left) {
                start_animation(p, (u32)Player_Anim::Punching_Right, false, 50);
                g.input.last_punch_was_left = false;
            } else {
                start_animation(p, (u32)Player_Anim::Punching_Left, false, 50);
                g.input.last_punch_was_left = true;
            }
        }
    }

    if (input_pressed(in.kick, in_prev.kick)) {
        if (!p->animation_playing || p->animation_loop) {
            if (g.input.last_kick_was_left) {
                start_animation(p, (u32)Player_Anim::Kicking_Right, false, 50);
                g.input.last_kick_was_left = false;
            } else {
                start_animation(p, (u32)Player_Anim::Kicking_Left, false, 50);
                g.input.last_kick_was_left = true;
            }
        }
    }

    bool is_moving = false;
    f32 x_vel = 0., y_vel = 0.;

    if (in.left) {
        x_vel -= p->speed;
        p->dir = Direction::Left;
        is_moving = true;
    }

    if (in.right) {
        x_vel += p->speed;
        p->dir = Direction::Right;
        is_moving = true;
    }

    if (in.up) {
        y_vel -= p->speed;
        is_moving = true;
    }

    if (in.down) {
        y_vel += p->speed;
        is_moving = true;
    }

    p->x += x_vel * g.dt;
    p->y += y_vel * g.dt;

    if (!p->animation_playing || p->animation_loop) {
        if (is_moving) {
            if (p->default_anim != (u32)Player_Anim::Running) {
                p->default_anim = (u32)Player_Anim::Running;
                start_animation(p, (u32)Player_Anim::Running, true);
            }
        } else {
            if (p->default_anim != (u32)Player_Anim::Standing) {
                p->default_anim = (u32)Player_Anim::Standing;
                start_animation(p, (u32)Player_Anim::Standing, true);
            }
        }
    }

    update_animation(p);
}

static void update() {
    for (u64 idx = 0; idx < g.enemies.size(); idx++) {
        update_enemy(&g.enemies[idx]);
    }
    update_player(&g.player);

    g.input_prev  = g.input;
    g.input.punch = false;
    g.input.kick  = false;
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

static void handle_input(const SDL_Event& e) {
    if (e.type == SDL_EVENT_KEY_DOWN) {
        switch (e.key.key) {
            case SDLK_LEFT:   g.input.left  = true; break;
            case SDLK_RIGHT:  g.input.right = true; break;
            case SDLK_UP:     g.input.up    = true; break;
            case SDLK_DOWN:   g.input.down  = true; break;
            case SDLK_RETURN: g.input.punch = true; break;
            case SDLK_SPACE:  g.input.kick  = true; break;
        }
    }

    if (e.type == SDL_EVENT_KEY_UP) {
        switch (e.key.key) {
            case SDLK_LEFT:  g.input.left  = false; break;
            case SDLK_RIGHT: g.input.right = false; break;
            case SDLK_UP:    g.input.up    = false; break;
            case SDLK_DOWN:  g.input.down  = false; break;
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
            update();
            draw();
        }
    }

    return 0;
}

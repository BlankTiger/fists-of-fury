#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <vector>
#include <cmath>

#include "number_types.h"
#include "sprite.h"
#include "level_info.h"
#include "settings.h"

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
    // this will be relative to the player position
    SDL_FRect collision_box_offsets;
    SDL_FRect shadow_offset;

    u32 current_frame      = 0;     // Current frame in the animation
    u64 last_frame_time    = 0;     // When the last frame was shown
    u64 frame_duration_ms  = 100;   // Milliseconds per frame
    bool animation_playing = false;
    bool animation_loop    = true;  // Whether this animation should loop
    u32 default_anim       = 0;     // Animation to return to when current finishes
};

enum struct Kick_State { Left, Right, Drop };

struct Input_State {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool punch = false;
    bool kick  = false;

    Kick_State last_kick     = Kick_State::Left;
    bool last_punch_was_left = true;
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

    Level_Info curr_level_info;
    SDL_FRect  camera;
    u64        dt;
};

static Game g = {};

enum struct Player_Anim : u32 {
    Standing       = 0,
    Running        = 1,
    Punching_Left  = 2,
    Punching_Right = 3,
    Kicking_Left   = 4,
    Kicking_Right  = 5,
    Kicking_Drop   = 6,
    Got_Hit        = 7,
    Fallover       = 8
};

internal void start_animation(Entity* e, u32 anim_idx, bool should_loop = false, u64 frame_time = 100) {
    e->idx_anim          = anim_idx;
    e->current_frame     = 0;
    e->animation_playing = true;
    e->animation_loop    = should_loop;
    e->frame_duration_ms = frame_time;
    e->last_frame_time   = SDL_GetTicks();
}

// internal bool is_animation_finished(const Entity& e) {
//     if (!e.animation_playing) return true;
//     if (e.animation_loop) return false;
//     return e.current_frame >= g.sprite_player.frames_in_each_row[e.idx_anim] - 1;
// }

internal void update_animation(Entity* e) {
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
        g.curr_level_info = level_data[0];
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

    {
        // player setup
        g.player.x                     = SCREEN_WIDTH  / 16;
        g.player.y                     = SCREEN_HEIGHT / 16;
        g.player.collision_box_offsets = {14, 40, 20, 8};
        g.player.shadow_offset         = {17, 48, 14, 2};
        g.player.default_anim          = (u32)Player_Anim::Standing;
        start_animation(&g.player, (u32)Player_Anim::Standing, true);
    }

    {
        // enemies setup
    }

    return true;
}

internal void draw_collision_box(SDL_Renderer* r, const SDL_FRect& box) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 255, 2, 0, 200);
    bool ok = SDL_RenderRect(r, &box);
    if (!ok) SDL_Log("Failed to draw background collision box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 150, 0, 0, 100);
    ok = SDL_RenderFillRect(r, &box);
    if (!ok) SDL_Log("Failed to draw background collision box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

internal void draw_level(SDL_Renderer* r) {
    const SDL_FRect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderTexture(r, g.bg.img, &g.camera, &dst);
    #if SHOW_COLLISION_BOXES
    for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
        // Draw collision boxes relative to camera
        SDL_FRect screen_box = {
            box.x - g.camera.x,
            box.y - g.camera.y,
            box.w,
            box.h
        };
        draw_collision_box(r, screen_box);
    }
    #endif
}

internal void draw_entity(SDL_Renderer* r, Entity e) {
    // Draw entity relative to camera position
    f32 screen_x = e.x - g.camera.x;
    f32 screen_y = e.y - g.camera.y;

    const SDL_FRect dst = {screen_x, screen_y, g.entity_shadow.width, g.entity_shadow.height};
    SDL_RenderTexture(r, g.entity_shadow.img, NULL, &dst);
    #if SHOW_COLLISION_BOXES
    // TODO: Add collision box drawing with camera offset
    #endif
}

internal void update_enemy(Entity* e) {
    // TODO: remove this
    e->health = e->health;
}

internal SDL_FRect player_get_offset_box(const Entity& p, const SDL_FRect& box) {
    return {
        box.x + p.x,
        box.y + p.y,
        box.w,
        box.h
    };
}

internal void draw_player(SDL_Renderer* r, const Entity& p) {
    // Draw player relative to camera position
    f32 screen_x = p.x - g.camera.x;
    f32 screen_y = p.y - g.camera.y;

    SDL_FlipMode flip = (p.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    bool ok = sprite_draw_at_dst(g.sprite_player, r, screen_x, screen_y, p.idx_anim, p.current_frame, flip);
    if (!ok) SDL_Log("Failed to draw player sprite! SDL err: %s\n", SDL_GetError());

    // Draw shadow at screen coordinates too
    SDL_FRect shadow_screen = {
        p.shadow_offset.x + screen_x,
        p.shadow_offset.y + screen_y,
        p.shadow_offset.w,
        p.shadow_offset.h
    };
    ok = SDL_RenderTexture(r, g.entity_shadow.img, NULL, &shadow_screen);

    #if SHOW_COLLISION_BOXES
    SDL_FRect collision_box = {
        p.collision_box_offsets.x + screen_x,
        p.collision_box_offsets.y + screen_y,
        p.collision_box_offsets.w,
        p.collision_box_offsets.h
    };
    draw_collision_box(r, collision_box);
    #endif
}

internal bool input_pressed(bool curr, bool prev) {
    return curr && !prev;
}

// internal bool input_released(bool curr, bool prev) {
//     return !curr && prev;
// }

internal void update_player(Entity* p) {
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
            switch (g.input.last_kick) {
                case Kick_State::Drop: {
                    start_animation(p, (u32)Player_Anim::Kicking_Drop, false, 80);
                    g.input.last_kick = Kick_State::Left;
                    break;
                }

                case Kick_State::Left: {
                    start_animation(p, (u32)Player_Anim::Kicking_Left, false, 50);
                    g.input.last_kick = Kick_State::Right;
                    break;
                }

                case Kick_State::Right: {
                    start_animation(p, (u32)Player_Anim::Kicking_Right, false, 50);
                    g.input.last_kick = Kick_State::Drop;
                    break;
                }
            }
        }
    }

    bool is_moving = false;
    f32 x_vel = 0., y_vel = 0.;

    if (in.left) {
        x_vel -= 1.0f;
        p->dir = Direction::Left;
        is_moving = true;
    }
    if (in.right) {
        x_vel += 1.0f;
        p->dir = Direction::Right;
        is_moving = true;
    }
    if (in.up) {
        y_vel -= 1.0f;
        is_moving = true;
    }
    if (in.down) {
        y_vel += 1.0f;
        is_moving = true;
    }

    // Normalize the velocity vector for diagonal movement
    if (is_moving) {
        f32 length = sqrt(x_vel * x_vel + y_vel * y_vel);
        if (length > 0.0f) {
            x_vel = (x_vel / length) * p->speed;
            y_vel = (y_vel / length) * p->speed;
        }
    }

    // handle collisions and position change
    {
        f32 x_old = p->x;
        f32 y_old = p->y;
        p->x += x_vel * g.dt;
        p->y += y_vel * g.dt;

        bool in_bounds = true;
        for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
            const SDL_FRect collision_box = player_get_offset_box(*p, p->collision_box_offsets);
            if (SDL_HasRectIntersectionFloat(&box, &collision_box)) {
                in_bounds = false;
                break;
            }
        }
        if (!in_bounds) {
            p->x = x_old;
            p->y = y_old;
        }
    }

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

void update_borders_for_curr_level() {
    auto new_left   = level_info_get_collision_box(g.curr_level_info, Border::Left);
    auto new_top    = level_info_get_collision_box(g.curr_level_info, Border::Top);
    auto new_bottom = level_info_get_collision_box(g.curr_level_info, Border::Bottom);
    new_left.x      = g.camera.x - 1;
    new_top.x       = g.camera.x - 1;
    new_bottom.x    = g.camera.x - 1;
    level_info_update_collision_box(g.curr_level_info, Border::Left,   new_left);
    level_info_update_collision_box(g.curr_level_info, Border::Top,    new_top);
    level_info_update_collision_box(g.curr_level_info, Border::Bottom, new_bottom);
}

const f32 w_half_screen = SCREEN_WIDTH / 2;

internal void update_camera(const Entity& player) {
    // Use the player's sprite center position
    f32 player_sprite_width = 48.;
    f32 x_player = player.x + (player_sprite_width / 2.);
    f32 player_screen_pos = x_player - g.camera.x;

    // Only move camera right when player crosses halfway point
    if (player_screen_pos > w_half_screen) {
        g.camera.x = x_player - w_half_screen;

        // this is to ensure that the player cant go back to the left,
        // and that borders on the top and bottom move with the player
        update_borders_for_curr_level();
    }

    // Clamp camera to level boundaries
    {
        if (g.camera.x < 0) {
            g.camera.x = 0;
        }

        f32 max_camera_x = g.bg.width - SCREEN_WIDTH;
        if (g.camera.x > max_camera_x) {
            g.camera.x = max_camera_x;
        }
    }
}

internal void update() {
    for (u64 idx = 0; idx < g.enemies.size(); idx++) {
        update_enemy(&g.enemies[idx]);
    }
    update_player(&g.player);
    update_camera(g.player);

    g.input_prev  = g.input;
    g.input.punch = false;
    g.input.kick  = false;
}

internal void draw() {
    SDL_RenderClear(g.renderer);

    draw_level(g.renderer);

    for (u64 idx = 0; idx < g.enemies.size(); idx++) {
        draw_entity(g.renderer, g.enemies[idx]);
    }
    draw_player(g.renderer, g.player);

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
            update();
            draw();
        }
    }

    return 0;
}

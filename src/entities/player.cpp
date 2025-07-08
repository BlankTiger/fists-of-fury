#include <cmath>

#include "player.h"
#include "../game.h"
#include "../settings.h"
#include "../draw.h"

void start_animation(Entity& e, u32 anim_idx, bool should_loop, u64 frame_time) {
    e.idx_anim          = anim_idx;
    e.current_frame     = 0;
    e.animation_playing = true;
    e.animation_loop    = should_loop;
    e.frame_duration_ms = frame_time;
    e.last_frame_time   = SDL_GetTicks();
}

// internal bool is_animation_finished(const Entity& e) {
//     if (!e.animation_playing) return true;
//     if (e.animation_loop) return false;
//     return e.current_frame >= g.sprite_player.frames_in_each_row[e.idx_anim] - 1;
// }

internal void update_animation(Entity& e, const Game& g) {
    if (!e.animation_playing) return;

    u64 current_time = SDL_GetTicks();
    if (current_time - e.last_frame_time >= e.frame_duration_ms) {
        e.current_frame++;
        e.last_frame_time = current_time;

        // Check if animation finished
        if (e.current_frame >= g.sprite_player.frames_in_each_row[e.idx_anim]) {
            if (e.animation_loop) {
                e.current_frame = 0; // Loop back to start
            } else {
                // Animation finished - return to default
                e.animation_playing = false;
                start_animation(e, e.default_anim, true);
            }
        }
    }
}

internal SDL_FRect player_get_offset_box(const Entity& p, const SDL_FRect& box) {
    return {
        box.x + p.x,
        box.y + p.y,
        box.w,
        box.h
    };
}

void update_borders_for_curr_level(Game& g) {
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

void update_camera(const Entity& player, Game& g) {
    // Use the player's sprite center position
    f32 player_sprite_width = 48.;
    f32 x_player = player.x + (player_sprite_width / 2.);
    f32 player_screen_pos = x_player - g.camera.x;

    // Only move camera right when player crosses halfway point
    if (player_screen_pos > w_half_screen) {
        g.camera.x = x_player - w_half_screen;

        // this is to ensure that the player cant go back to the left,
        // and that borders on the top and bottom move with the player
        update_borders_for_curr_level(g);
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

internal bool input_pressed(bool curr, bool prev) {
    return curr && !prev;
}

// internal bool input_released(bool curr, bool prev) {
//     return !curr && prev;
// }

void update_player(Entity& p, Game& g) {
    const auto& in      = g.input;
    const auto& in_prev = g.input_prev;

    if (input_pressed(in.punch, in_prev.punch)) {
        if (!p.animation_playing || p.animation_loop) {
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
        if (!p.animation_playing || p.animation_loop) {
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
        p.dir = Direction::Left;
        is_moving = true;
    }
    if (in.right) {
        x_vel += 1.0f;
        p.dir = Direction::Right;
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
            x_vel = (x_vel / length) * p.speed;
            y_vel = (y_vel / length) * p.speed;
        }
    }

    // handle collisions and position change
    {
        f32 x_old = p.x;
        f32 y_old = p.y;
        p.x += x_vel * g.dt;
        p.y += y_vel * g.dt;

        bool in_bounds = true;
        for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
            const SDL_FRect collision_box = player_get_offset_box(p, p.collision_box_offsets);
            if (SDL_HasRectIntersectionFloat(&box, &collision_box)) {
                in_bounds = false;
                break;
            }
        }
        if (!in_bounds) {
            p.x = x_old;
            p.y = y_old;
        }
    }

    if (!p.animation_playing || p.animation_loop) {
        if (is_moving) {
            if (p.default_anim != (u32)Player_Anim::Running) {
                p.default_anim = (u32)Player_Anim::Running;
                start_animation(p, (u32)Player_Anim::Running, true);
            }
        } else {
            if (p.default_anim != (u32)Player_Anim::Standing) {
                p.default_anim = (u32)Player_Anim::Standing;
                start_animation(p, (u32)Player_Anim::Standing, true);
            }
        }
    }

    update_animation(p, g);
}

void draw_player(SDL_Renderer* r, const Entity& p, const Game& g) {
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

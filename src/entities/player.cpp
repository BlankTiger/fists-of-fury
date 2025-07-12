#include <cmath>

#include "player.h"
#include "../game.h"
#include "../settings.h"
#include "../draw.h"

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

void update_borders_for_curr_level(Game& g) {
    auto new_left   = level_info_get_collision_box(g.curr_level_info, Border::Left);
    auto new_top    = level_info_get_collision_box(g.curr_level_info, Border::Top);
    auto new_bottom = level_info_get_collision_box(g.curr_level_info, Border::Bottom);
    // -1 to be 1 pixel off the screen to the left
    new_left.x      = g.camera.x - 1;
    new_top.x       = g.camera.x - 1;
    new_bottom.x    = g.camera.x - 1;
    level_info_update_collision_box(g.curr_level_info, Border::Left,   new_left);
    level_info_update_collision_box(g.curr_level_info, Border::Top,    new_top);
    level_info_update_collision_box(g.curr_level_info, Border::Bottom, new_bottom);
}

const f32 w_half_screen = SCREEN_WIDTH / 2;

internal void camera_update(const Entity& player, Game& g) {
    f32 player_screen_pos = player.x - g.camera.x;

    // Only move camera right when player crosses halfway point
    if (player_screen_pos > w_half_screen) {
        g.camera.x = player.x - w_half_screen;

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

void player_update(Entity& p, Game& g) {
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
        SDL_FRect player_collision_box = entity_get_world_collision_box(p);

        for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
            if (SDL_HasRectIntersectionFloat(&box, &player_collision_box)) {
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
    camera_update(p, g);
}

void player_draw(SDL_Renderer* r, const Entity& p, const Game& g) {
    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(p);
    const Vec2<f32> screen_coords = game_get_screen_coords(g, drawing_coords);

    const SDL_FlipMode flip = (p.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    bool ok = sprite_draw_at_dst(
        g.sprite_player,
        r,
        screen_coords.x,
        screen_coords.y,
        p.idx_anim,
        p.current_frame,
        flip
    );
    if (!ok) SDL_Log("Failed to draw player sprite! SDL err: %s\n", SDL_GetError());

    const Vec2<f32> world_coords = {p.x, p.y};
    draw_shadow(r, world_coords, p.shadow_offsets, g);
    #if SHOW_COLLISION_BOXES
    draw_collision_box(r, world_coords, p.collision_box_offsets, g);
    #endif
}

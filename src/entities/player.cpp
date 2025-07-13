#include <cmath>
#include <cassert>

#include "player.h"
#include "../game.h"
#include "../settings.h"
#include "../draw.h"

Entity player_init() {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity player{};
    player.type                      = Entity_Type::Player;
    player.x                         = 20;
    player.y                         = 50;
    player.sprite_frame_w            = sprite_frame_w;
    player.sprite_frame_h            = sprite_frame_h;
    player.collision_box_offsets     = {-sprite_frame_w/7, -4, 2*sprite_frame_w/7, 4};
    player.hurtbox_offsets           = {sprite_frame_w/7, -14, 10, 6};
    player.hitbox_offsets            = {-sprite_frame_w/6.5f, -23, 2*sprite_frame_w/6.5f, 23};
    player.shadow_offsets            = {-7, -1, 14, 2};
    player.extra_player.state        = Player_State::Standing;
    start_animation(player, (u32)Player_Anim::Standing, true);
    return player;
}


internal bool is_animation_finished(const Entity& e, const Game& g) {
    if (!e.animation_playing) return true;
    if (e.animation_loop) return false;
    return e.current_frame >= g.sprite_player.frames_in_each_row[e.idx_anim] - 1;
}

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
                e.animation_playing = false;
                // loop the last frame until we start another animation
                e.current_frame -= 1;
            }
        }
    }
}

internal void update_borders_for_curr_level(Game& g) {
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

enum struct Action {
    Left,
    Right,
    Up,
    Down,
    Punch,
    Kick,
    Jump
};

internal bool is_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Punch: {
            return g.input.punch;
            break;
        }

        case Action::Kick: {
            return g.input.kick;
            break;
        }

        case Action::Left: {
            return g.input.left;
            break;
        }

        case Action::Right: {
            return g.input.right;
            break;
        }

        case Action::Up: {
            return g.input.up;
            break;
        }

        case Action::Down: {
            return g.input.down;
            break;
        }

        case Action::Jump: {
            return g.input.jump;
            break;
        }
    }

    return false;
}

internal bool just_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Punch: {
            return input_pressed(g.input.punch, g.input_prev.punch);
            break;
        }

        case Action::Kick: {
            return input_pressed(g.input.kick, g.input_prev.kick);
            break;
        }

        case Action::Left: {
            return input_pressed(g.input.left, g.input_prev.left);
            break;
        }

        case Action::Right: {
            return input_pressed(g.input.right, g.input_prev.right);
            break;
        }

        case Action::Up: {
            return input_pressed(g.input.up, g.input_prev.up);
            break;
        }

        case Action::Down: {
            return input_pressed(g.input.down, g.input_prev.down);
            break;
        }

        case Action::Jump: {
            return input_pressed(g.input.jump, g.input_prev.jump);
            break;
        }
    }

    return false;
}

internal bool not_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Left: {
            return !g.input.left;
            break;
        }

        case Action::Right: {
            return !g.input.right;
            break;
        }

        case Action::Up: {
            return !g.input.up;
            break;
        }

        case Action::Down: {
            return !g.input.down;
            break;
        }

        case Action::Punch: {
            return !g.input.punch;
            break;
        }

        case Action::Kick: {
            return !g.input.kick;
            break;
        }

        case Action::Jump: {
            return !g.input.jump;
            break;
        }
    }

    return true;
}

internal bool started_moving(const Game& g) {
    return is_pressed(g, Action::Up)
        || is_pressed(g, Action::Down)
        || is_pressed(g, Action::Left)
        || is_pressed(g, Action::Right);
}

internal bool stopped_moving(const Game& g) {
    return not_pressed(g, Action::Up)
        && not_pressed(g, Action::Down)
        && not_pressed(g, Action::Left)
        && not_pressed(g, Action::Right);
}

internal void handle_movement(Entity& p, const Game& g) {
    const auto in = g.input;

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

        if (in_bounds) {
            for (const auto& e : g.entities) {
                if (e.type == Entity_Type::Player) continue;

                const auto& e_box = entity_get_world_collision_box(e);
                if (SDL_HasRectIntersectionFloat(&e_box, &player_collision_box)) {
                    in_bounds = false;
                    break;
                }
            }
        }

        if (!in_bounds) {
            p.x = x_old;
            p.y = y_old;
        }
    }

    // rotate the hurtbox around the player when turning
    switch (p.dir) {
        case Direction::Left: {
            p.hurtbox_offsets.x = -std::fabs(p.hurtbox_offsets.x);
            p.hurtbox_offsets.w = -std::fabs(p.hurtbox_offsets.w);
            break;
        }

        case Direction::Right: {
            p.hurtbox_offsets.x = std::fabs(p.hurtbox_offsets.x);
            p.hurtbox_offsets.w = std::fabs(p.hurtbox_offsets.w);
            break;
        }

        case Direction::Up: break;
        case Direction::Down: break;
    }
}


internal void player_kick(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Kicking;
    switch (g.input.last_kick) {
        case Kick_State::Right: {
            start_animation(p, (u32)Player_Anim::Kicking_Right, false, 80);
            g.input.last_kick = Kick_State::Left;
            break;
        }

        case Kick_State::Left: {
            start_animation(p, (u32)Player_Anim::Kicking_Left, false, 50);
            g.input.last_kick = Kick_State::Right;
            break;
        }
    }
}

internal void player_punch(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Punching;
    if (g.input.last_punch_was_left) {
        start_animation(p, (u32)Player_Anim::Punching_Right, false, 60);
        g.input.last_punch_was_left = false;
    } else {
        start_animation(p, (u32)Player_Anim::Punching_Left, false, 60);
        g.input.last_punch_was_left = true;
    }
}

internal void player_takeoff(Entity& p) {
    p.extra_player.state = Player_State::Takeoff;
    p.z_vel = JUMP_VELOCITY;
    start_animation(p, (u32)Player_Anim::Takeoff, false, 200);
}

internal void player_jump(Entity& p) {
    p.extra_player.state = Player_State::Jumping;
    start_animation(p, (u32)Player_Anim::Jumping, true);
}

internal void player_land(Entity& p) {
    p.extra_player.state = Player_State::Landing;
    start_animation(p, (u32)Player_Anim::Landing, false);
}

internal void player_drop_kick(Entity& p) {
    p.extra_player.state = Player_State::Kicking_Drop;
    start_animation(p, (u32)Player_Anim::Kicking_Drop, false, 80);
}

internal void player_stand(Entity& p) {
    p.extra_player.state = Player_State::Standing;
    start_animation(p, (u32)Player_Anim::Standing, true);
}

internal void player_run(Entity& p) {
    p.extra_player.state = Player_State::Running;
    start_animation(p, (u32)Player_Anim::Running, true);
}

internal void handle_jump_physics(Entity& p, const Game& g) {
    p.z_vel += GRAVITY * g.dt;
    p.z += p.z_vel * g.dt;

    // remember that this is reversed (up means negative, down means positive)
    if (p.z >= GROUND_LEVEL) {
        p.z = GROUND_LEVEL;
        p.z_vel = 0.0f;
        if (p.extra_player.state == Player_State::Jumping) {
            player_land(p);
        }
    }
}

void player_update(Entity& p, Game& g) {
    assert(p.type == Entity_Type::Player);

    switch (p.extra_player.state) {
        case Player_State::Standing: {
            if (started_moving(g)) {
                player_run(p);
                handle_movement(p, g);
            }
            else if (just_pressed(g, Action::Punch)) {
                player_punch(p, g);
            }
            else if (just_pressed(g, Action::Kick)) {
                player_kick(p, g);
            }
            else if (just_pressed(g, Action::Jump)) {
                player_takeoff(p);
            }

            break;
        }

        case Player_State::Running: {
            if (stopped_moving(g)) {
                player_stand(p);
            }
            else if (just_pressed(g, Action::Punch)) {
                player_punch(p, g);
            }
            else if (just_pressed(g, Action::Kick)) {
                player_kick(p, g);
            }
            else if (just_pressed(g, Action::Jump)) {
                player_takeoff(p);
            }
            else {
                handle_movement(p, g);
            }

            break;
        }

        case Player_State::Punching: {
            // deal damage if in hitbox

            if (is_animation_finished(p, g)) {
                player_stand(p);
            }

            break;
        }

        case Player_State::Kicking: {
            // deal damage if in hitbox

            if (is_animation_finished(p, g)) {
                player_stand(p);
            }

            break;
        };

        case Player_State::Got_Hit: {
            assert(false); // unimplemented
            break;
        }

        case Player_State::Dying: {
            assert(false); // unimplemented
            break;
        }

        case Player_State::Takeoff: {
            if (is_animation_finished(p, g)) {
                player_jump(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Jumping: {
            if (just_pressed(g, Action::Kick)) {
                player_drop_kick(p);
            }
            else if (is_animation_finished(p, g)) {
                player_land(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Landing: {
            if (is_animation_finished(p, g)) {
                player_stand(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Kicking_Drop: {
            // deal damage if in hitbox

            if (p.z == GROUND_LEVEL) {
                player_stand(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }
    }

    update_animation(p, g);
    camera_update(p, g);
}

void player_draw(SDL_Renderer* r, const Entity& p, const Game& g) {
    assert(p.type == Entity_Type::Player);

    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(p);
    Vec2<f32> screen_coords = game_get_screen_coords(g, drawing_coords);
    screen_coords.y += p.z; // for jumping

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

    Vec2<f32> world_coords = {p.x, p.y};
    draw_shadow(r, world_coords, p.shadow_offsets, g);

    // drawing debug *box
    {
        #if SHOW_COLLISION_BOXES || DEV_MODE
        draw_collision_box(r, world_coords, p.collision_box_offsets, g);
        #endif

        // this is so that both hurtbox and hitbox go along with the player when he jumps
        world_coords.y += p.z;
        #if SHOW_HURTBOXES || DEV_MODE
        draw_hurtbox(r, world_coords, p.hurtbox_offsets, g);
        #endif

        #if SHOW_HITBOXES || DEV_MODE
        draw_hitbox(r, world_coords, p.hitbox_offsets, g);
        #endif
    }
}

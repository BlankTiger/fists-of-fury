#include <cmath>
#include <cassert>
#include <iostream>

#include "player.h"
#include "../game.h"
#include "../settings.h"
#include "../draw.h"

Entity player_init(const Sprite* player_sprite) {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity player{};
    player.health                    = 100;
    player.damage                    = 20;
    player.speed                     = 0.03;
    player.type                      = Entity_Type::Player;
    player.x                         = 20;
    player.y                         = 50;
    player.sprite_frame_w            = sprite_frame_w;
    player.sprite_frame_h            = sprite_frame_h;
    player.collision_box_offsets     = {-sprite_frame_w/7, -3, 2*sprite_frame_w/7, 4};
    player.hurtbox_offsets           = {sprite_frame_w/7, -14, 10, 6};
    player.hitbox_offsets            = {-sprite_frame_w/6.5f, -23, 2*sprite_frame_w/6.5f, 23};
    player.shadow_offsets            = {-7, -1, 14, 2};
    player.extra_player.state        = Player_State::Standing;
    player.anim.sprite               = player_sprite;
    animation_start(player.anim, { .anim_idx = (u32)Player_Anim::Standing, .looping = true});
    return player;
}


static void update_borders_for_curr_level(Game& g) {
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

static void camera_update(const Entity& player, Game& g) {
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

static bool input_pressed(bool curr, bool prev) {
    return curr && !prev;
}

// static bool input_released(bool curr, bool prev) {
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

static bool is_pressed(const Game& g, Action a) {
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

static bool just_pressed(const Game& g, Action a) {
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

static bool not_pressed(const Game& g, Action a) {
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

static bool started_moving(const Game& g) {
    return is_pressed(g, Action::Up)
        || is_pressed(g, Action::Down)
        || is_pressed(g, Action::Left)
        || is_pressed(g, Action::Right);
}

static bool stopped_moving(const Game& g) {
    return not_pressed(g, Action::Up)
        && not_pressed(g, Action::Down)
        && not_pressed(g, Action::Left)
        && not_pressed(g, Action::Right);
}

static void handle_movement(Entity& p, const Game& g) {
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
    if (p.dir != p.dir_prev) {
        if ((p.dir_prev == Direction::Right && p.dir == Direction::Left) ||
            (p.dir_prev == Direction::Left  && p.dir == Direction::Right)) {
            p.hurtbox_offsets.x = -p.hurtbox_offsets.x - p.hurtbox_offsets.w;
        }
    }

    p.dir_prev = p.dir;
}

static void handle_player_attack(Entity& p, Game& g) {
    for (auto& e : g.entities) {
        if (e.type == Entity_Type::Player) continue;

        SDL_FRect player_hurtbox = entity_get_world_hurtbox(p);
        SDL_FRect entity_hitbox = entity_get_world_hitbox(e);
        if (SDL_HasRectIntersectionFloat(&entity_hitbox, &player_hurtbox)) {
            e.damage_queue.push_back({(f32)p.damage, {}});
        }
    }
}

static void player_kick(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Kicking;
    switch (g.input.last_kick) {
        case Kick_State::Right: {
            animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Kicking_Right, .looping = false, .frame_duration_ms = 80});
            g.input.last_kick = Kick_State::Left;
            break;
        }

        case Kick_State::Left: {
            animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Kicking_Left, .looping = false, .frame_duration_ms = 50});
            g.input.last_kick = Kick_State::Right;
            break;
        }
    }

    handle_player_attack(p, g);
}

static void player_punch(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Punching;
    if (g.input.last_punch_was_left) {
        animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Punching_Right, .looping = false, .frame_duration_ms = 60});
        g.input.last_punch_was_left = false;
    } else {
        animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Punching_Left, .looping = false, .frame_duration_ms = 60});
        g.input.last_punch_was_left = true;
    }

    handle_player_attack(p, g);
}

static void player_takeoff(Entity& p) {
    p.extra_player.state = Player_State::Takeoff;
    p.z_vel = settings.jump_velocity;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Takeoff, .looping = false, .frame_duration_ms = 200});
}

static void player_jump(Entity& p) {
    p.extra_player.state = Player_State::Jumping;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Jumping, .looping = true});
}

static void player_land(Entity& p) {
    p.extra_player.state = Player_State::Landing;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Landing, .looping = false});
}

static void player_drop_kick(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Kicking_Drop;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Kicking_Drop, .looping =  false, .frame_duration_ms = 80});
    handle_player_attack(p, g);
}

static void player_stand(Entity& p) {
    p.extra_player.state = Player_State::Standing;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Standing, .looping = true });
}

static void player_run(Entity& p) {
    p.extra_player.state = Player_State::Running;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Running, .looping = true});
}

static void handle_jump_physics(Entity& p, const Game& g) {
    p.z_vel += settings.gravity * g.dt;
    p.z += p.z_vel * g.dt;

    // remember that this is reversed (up means negative, down means positive)
    if (p.z >= settings.ground_level) {
        p.z = settings.ground_level;
        p.z_vel = 0.0f;
        if (p.extra_player.state == Player_State::Jumping) {
            player_land(p);
        }
    }
}

Update_Result player_update(Entity& p, Game& g) {
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

            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }

            break;
        }

        case Player_State::Kicking: {
            // deal damage if in hitbox

            if (animation_is_finished(p.anim)) {
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
            if (animation_is_finished(p.anim)) {
                player_jump(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Jumping: {
            if (just_pressed(g, Action::Kick)) {
                player_drop_kick(p, g);
            }
            else if (animation_is_finished(p.anim)) {
                player_land(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Landing: {
            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }

        case Player_State::Kicking_Drop: {
            // deal damage if in hitbox

            if (p.z == settings.ground_level) {
                player_stand(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

            break;
        }
    }

    animation_update(p.anim);
    camera_update(p, g);

    return Update_Result::None;
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
        p.anim.frames.idx,
        p.anim.frames.frame_current,
        flip
    );
    if (!ok) SDL_Log("Failed to draw player sprite! SDL err: %s\n", SDL_GetError());

    Vec2<f32> world_coords = {p.x, p.y};
    draw_shadow(r, world_coords, p.shadow_offsets, g);

    // drawing debug *box
    {
        if (settings.show_collision_boxes) draw_collision_box(r, world_coords, p.collision_box_offsets, g);

        // this is so that both hurtbox and hitbox go along with the player when he jumps
        world_coords.y += p.z;
        if (settings.show_hurtboxes) draw_hurtbox(r, world_coords, p.hurtbox_offsets, g);

        if (settings.show_hitboxes) draw_hitbox(r, world_coords, p.hitbox_offsets, g);
    }
}

#include <cmath>
#include <cassert>

#include "player.h"
#include "knife.h"
#include "../game.h"
#include "../settings.h"
#include "../draw.h"
#include "../utils.h"

Entity player_init(const Sprite* player_sprite, Game& g) {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity player{};
    player.handle                = game_generate_entity_handle(g);
    player.health                = 200.0f;
    player.damage                = 20;
    player.speed                 = 0.03f;
    player.type                  = Entity_Type::Player;
    player.x                     = 20;
    player.y                     = 50;
    player.dir                   = Direction::Right;
    player.sprite_frame_w        = sprite_frame_w;
    player.sprite_frame_h        = sprite_frame_h;
    player.collision_box_offsets = {-sprite_frame_w/7,    -3,  2*sprite_frame_w/7,    4};
    player.hurtbox_offsets       = {sprite_frame_w/7,     -16, 10,                    6};
    player.hitbox_offsets        = {-sprite_frame_w/6.5f, -23, 2*sprite_frame_w/6.5f, 15};
    player.shadow_offsets        = {-7,                   -1,  14,                    2};
    player.anim.sprite           = player_sprite;
    player.extra_player.state    = Player_State::Standing;
    player.extra_player.slots    = {
        .offset_top_left     = {-sprite_frame_w/3.0f, -4.0f},
        .offset_top_right    = {sprite_frame_w/3.0f,  -4.0f},
        .offset_bottom_left  = {-sprite_frame_w/4,    2},
        .offset_bottom_right = {sprite_frame_w/4,     2},
        .top_left_free       = true,
        .top_right_free      = true,
        .bottom_left_free    = true,
        .bottom_right_free   = true,
    };
    player.extra_player.has_knife = true;
    animation_start(player.anim, { .anim_idx = (u32)Player_Anim::Standing, .looping = true});
    return player;
}


static void update_borders_for_curr_level(Game& g) {
    auto new_left   = level_info_get_collision_box(g.curr_level_info, Border::Left);
    auto new_top    = level_info_get_collision_box(g.curr_level_info, Border::Top);
    auto new_bottom = level_info_get_collision_box(g.curr_level_info, Border::Bottom);
    auto new_right  = level_info_get_collision_box(g.curr_level_info, Border::Right);
    // -1 to be 1 pixel off the screen to the left
    new_left.x      = g.camera.x - 1;
    new_top.x       = g.camera.x - 1;
    new_bottom.x    = g.camera.x - 1;
    new_right.x     = g.camera.x + SCREEN_WIDTH;
    level_info_update_collision_box(g.curr_level_info, Border::Left,   new_left);
    level_info_update_collision_box(g.curr_level_info, Border::Top,    new_top);
    level_info_update_collision_box(g.curr_level_info, Border::Bottom, new_bottom);
    level_info_update_collision_box(g.curr_level_info, Border::Right,  new_right);
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

enum struct Action {
    Left,
    Right,
    Up,
    Down,
    Attack,
    Interact,
    Jump
};

static bool is_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Attack: {
            return g.input.attack;
        } break;

        case Action::Interact: {
            return g.input.interact;
        } break;

        case Action::Left: {
            return g.input.left;
        } break;

        case Action::Right: {
            return g.input.right;
        } break;

        case Action::Up: {
            return g.input.up;
        } break;

        case Action::Down: {
            return g.input.down;
        } break;

        case Action::Jump: {
            return g.input.jump;
        } break;
    }

    return false;
}

static bool just_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Attack: {
            return input_pressed(g.input.attack, g.input_prev.attack);
        } break;

        case Action::Interact: {
            return input_pressed(g.input.interact, g.input_prev.interact);
        } break;

        case Action::Left: {
            return input_pressed(g.input.left, g.input_prev.left);
        } break;

        case Action::Right: {
            return input_pressed(g.input.right, g.input_prev.right);
        } break;

        case Action::Up: {
            return input_pressed(g.input.up, g.input_prev.up);
        } break;

        case Action::Down: {
            return input_pressed(g.input.down, g.input_prev.down);
        } break;

        case Action::Jump: {
            return input_pressed(g.input.jump, g.input_prev.jump);
        } break;
    }

    return false;
}

static bool not_pressed(const Game& g, Action a) {
    switch (a) {
        case Action::Left: {
            return !g.input.left;
        } break;

        case Action::Right: {
            return !g.input.right;
        } break;

        case Action::Up: {
            return !g.input.up;
        } break;

        case Action::Down: {
            return !g.input.down;
        } break;

        case Action::Attack: {
            return !g.input.attack;
        } break;

        case Action::Interact: {
            return !g.input.interact;
        } break;

        case Action::Jump: {
            return !g.input.jump;
        } break;
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

    // for some reason there was leftover 0.03 when
    // we werent doing an explicit reset of these
    p.x_vel = 0.0f;
    p.y_vel = 0.0f;

    if (in.left) {
        p.x_vel -= 1.0f;
        p.dir = Direction::Left;
        is_moving = true;
    }
    if (in.right) {
        p.x_vel += 1.0f;
        p.dir = Direction::Right;
        is_moving = true;
    }
    if (in.up) {
        p.y_vel -= 1.0f;
        is_moving = true;
    }
    if (in.down) {
        p.y_vel += 1.0f;
        is_moving = true;
    }

    // Normalize the velocity vector for diagonal movement
    if (is_moving) {
        f32 length = sqrt(p.x_vel * p.x_vel + p.y_vel * p.y_vel);
        if (length > 0.0f) {
            p.x_vel = (p.x_vel / length) * p.speed;
            p.y_vel = (p.y_vel / length) * p.speed;
        }
    }

    static constexpr Entity_Type dont_collide_with[] = {Entity_Type::Enemy, Entity_Type::Collectible};
    static const Collide_Opts collide_opts = {
        .dont_collide_with = std::span{dont_collide_with},
        .collide_with_walls = false,
    };
    entity_movement_handle_collisions_and_pos_change(p, &g, collide_opts);

    entity_handle_rotating_hurtbox(p);
}

static void handle_attack(Entity& p, Game& g, Hit_Type type = Hit_Type::Normal) {
    SDL_FRect player_hurtbox = entity_get_world_hurtbox(p);
    bool attack_success = false;

    for (auto& e : g.entities) {
        if (e.type == Entity_Type::Player) continue;

        SDL_FRect entity_hitbox = entity_get_world_hitbox(e);
        if (SDL_HasRectIntersectionFloat(&entity_hitbox, &player_hurtbox)) {
            attack_success = true;
            e.damage_queue.push_back({(f32)p.damage, p.dir, type});
        }
    }

    p.extra_player.last_attack_successful = attack_success;
    if (attack_success) {
        p.extra_player.combo++;
        p.extra_player.last_attack_timestamp = g.time_ms;
    }
    else {
        p.extra_player.combo = 0;
    }
}

const u32 AMOUNT_OF_ATTACKS = 4;

// make this player_attack and then swap animations on combo
static void player_attack(Entity& p, Game& g) {
    u32 attack_anim        = (u32)Player_Anim::Punching_Right;
    Anim_Start_Opts opts   = {};
    opts.frame_duration_ms = 60;

    if (p.extra_player.has_knife) {
        p.extra_player.state = Player_State::Attacking;
        opts.anim_idx = (u32)Player_Anim::Punching_Right;
        animation_start(p.anim, opts);
        knife_throw(g, p);
        p.extra_player.has_knife = false;
        return;
    }

    auto should_be = p.extra_player.combo % AMOUNT_OF_ATTACKS;
    auto type = Hit_Type::Normal;
    switch (should_be) {
        case 0: {
            attack_anim = (u32)Player_Anim::Punching_Right;
        } break;

        case 1: {
            attack_anim = (u32)Player_Anim::Punching_Left;
        } break;

        case 2: {
            attack_anim = (u32)Player_Anim::Kicking_Left;
        } break;

        case 3: {
            attack_anim = (u32)Player_Anim::Kicking_Right;
            opts.frame_duration_ms = 45;
            type = Hit_Type::Power;
        } break;
    }

    opts.anim_idx = attack_anim;
    animation_start(p.anim, opts);
    p.extra_player.state = Player_State::Attacking;
    handle_attack(p, g, type);
}

static void player_takeoff(Entity& p) {
    p.extra_player.state = Player_State::Takeoff;
    p.z_vel = settings.jump_velocity;
    animation_start(
        p.anim,
        {
            .anim_idx = (u32)Player_Anim::Takeoff,
            .frame_duration_ms = 100
        }
    );
}

static void player_jump(Entity& p) {
    p.extra_player.state = Player_State::Jumping;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Jumping, .looping = true});
}

static void player_land(Entity& p) {
    p.extra_player.state = Player_State::Landing;
    animation_start(
        p.anim,
        {
            .anim_idx = (u32)Player_Anim::Landing,
            .frame_duration_ms = 200
        }
    );
}

static void player_drop_kick(Entity& p, Game& g) {
    p.extra_player.state = Player_State::Kicking_Drop;
    animation_start(
        p.anim,
        {
            .anim_idx          = (u32)Player_Anim::Kicking_Drop,
            .frame_duration_ms = 80,
            .looping           = false,
        }
    );
    handle_attack(p, g, Hit_Type::Knockdown);
}

static void player_stand(Entity& p) {
    p.extra_player.state = Player_State::Standing;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Standing, .looping = true });
}

static void player_run(Entity& p) {
    p.extra_player.state = Player_State::Running;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Running, .looping = true});
}

static void player_pick_up_collectible(Entity& p) {
    p.extra_player.state = Player_State::Picking_Up_Collectible;
    animation_start(p.anim, { .anim_idx = (u32)Player_Anim::Landing });
}

static void player_die(Entity& p) {
    p.extra_player.state = Player_State::Dying;
    animation_start(
        p.anim,
        { 
            .anim_idx = (u32)Player_Anim::Knocked_Down,
            .fadeout = {
                .enabled = true,
                .perc_per_sec = 0.5f,
            }
        }
    );
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

static bool player_can_receive_damage(const Entity& p) {
    const auto& s = p.extra_player.state;
    return s != Player_State::Takeoff
        && s != Player_State::Jumping
        && s != Player_State::Kicking_Drop
        && s != Player_State::Got_Hit
        && s != Player_State::Dying
        && s != Player_State::Landing;
}

static Anim_Start_Opts player_get_anim_got_hit() {
    Anim_Start_Opts opts = {};
    opts.anim_idx = (u32)Player_Anim::Got_Hit;
    opts.frame_duration_ms = 50;
    return opts;
}

static void player_pick_up(Entity& p, Game& g) {
    if (p.extra_player.has_knife) return;

    bool picked_something_up = false;
    auto collectible = entity_pickup_collectible(p, g);
    if (collectible) {
        assert(collectible->type == Entity_Type::Collectible);

        switch (collectible->extra_collectible.type) {
            case Collectible_Type::Knife: {
                collectible->extra_collectible.picked_up = true;
                p.extra_player.has_knife = true;
                picked_something_up = true;
            } break;
        }
    }

    if (picked_something_up) {
        player_pick_up_collectible(p);
    }
}

static void player_receive_damage(Entity& p, Game& g) {
    if (p.health <= 0.0f) return;

    bool got_hit = false;
    Dmg most_significant_dmg = {};
    for (auto dmg : p.damage_queue) {
        got_hit = true;
        p.health -= dmg.amount;
        p.y_vel = 0.0f;
        if (dmg.going_to == Direction::Left) {
            p.x_vel = -settings.player_knockback_velocity;
        }
        else if (dmg.going_to == Direction::Right) {
            p.x_vel = settings.player_knockback_velocity;
        }
        else {
            unreachable("shouldnt ever get a different direction");
        }

        if (dmg.type != Hit_Type::Normal) {
            most_significant_dmg = dmg;
        }
    }

    if (got_hit) {
        p.extra_player.state = Player_State::Got_Hit;
        switch (most_significant_dmg.type) {
            case Hit_Type::Normal: {
                auto opts = player_get_anim_got_hit();
                animation_start(p.anim, opts);
            } break;

            case Hit_Type::Knockdown: {
                p.z_vel = -settings.player_knockdown_velocity;

                if (most_significant_dmg.going_to == Direction::Left) {
                    p.x_vel = -settings.player_knockdown_velocity;
                }
                else if (most_significant_dmg.going_to == Direction::Right) {
                    p.x_vel = settings.player_knockdown_velocity;
                }

                p.extra_player.state = Player_State::Got_Hit;
                auto opts = player_get_anim_got_hit();
                animation_start(p.anim, opts);
            } break;

            case Hit_Type::Power: {
                if (most_significant_dmg.going_to == Direction::Left) {
                    p.x_vel = -settings.player_flying_back_velocity;
                }
                else if (most_significant_dmg.going_to == Direction::Right) {
                    p.x_vel = settings.player_flying_back_velocity;
                }

                p.extra_player.state = Player_State::Got_Hit;
                auto opts = player_get_anim_got_hit();
                animation_start(p.anim, opts);
            } break;
        }

        if (p.extra_player.has_knife) {
            knife_drop(g, p, { .instantly_disappear = true });
            p.extra_player.has_knife = false;
        }
    }

    p.damage_queue.clear();
    return;
}

Update_Result player_update(Entity& p, Game& g) {
    assert(p.type == Entity_Type::Player);

    if (p.extra_player.combo > 0) {
        if (g.time_ms - p.extra_player.last_attack_timestamp > settings.player_combo_timeout_ms) {
            p.extra_player.combo = 0;
        }
    }

    if (player_can_receive_damage(p)) {
        player_receive_damage(p, g);
    }

    switch (p.extra_player.state) {
        case Player_State::Standing: {
            if (started_moving(g)) {
                player_run(p);
                handle_movement(p, g);
            } else if (just_pressed(g, Action::Attack)) {
                player_attack(p, g);
            } else if (just_pressed(g, Action::Interact)) {
                player_pick_up(p, g);
            } else if (just_pressed(g, Action::Jump)) {
                player_takeoff(p);
            }
        } break;

        case Player_State::Running: {
            if (stopped_moving(g)) {
                player_stand(p);
            } else if (just_pressed(g, Action::Attack)) {
                player_attack(p, g);
            } else if (just_pressed(g, Action::Jump)) {
                player_takeoff(p);
            } else {
                handle_movement(p, g);
            }
        } break;

        case Player_State::Attacking: {
            // deal damage if in hitbox

            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }
        } break;

        case Player_State::Got_Hit: {
            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }

            if (p.health <= 0.0f) {
                player_die(p);
            }
        } break;

        case Player_State::Dying: {
            if (animation_is_finished(p.anim)) {
                return Update_Result::Remove_Me;
            }
        } break;

        case Player_State::Takeoff: {
            if (animation_is_finished(p.anim)) {
                player_jump(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

        } break;

        case Player_State::Jumping: {
            if (just_pressed(g, Action::Attack)) {
                player_drop_kick(p, g);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

        } break;

        case Player_State::Landing: {
            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }

            handle_jump_physics(p, g);
        } break;

        case Player_State::Kicking_Drop: {
            if (p.z == settings.ground_level) {
                player_land(p);
            }

            handle_movement(p, g);
            handle_jump_physics(p, g);

        } break;

        case Player_State::Picking_Up_Collectible: {
            if (animation_is_finished(p.anim)) {
                player_stand(p);
            }
        }
    }

    animation_update(p.anim, g.dt, g.dt_real);
    camera_update(p, g);

    return Update_Result::None;
}

static void slots_draw(SDL_Renderer* r, const Entity& p, const Game& g) {
    const auto& slots = p.extra_player.slots;
    // TODO: maybe consider making the entity position being Vec2 instead of doing that all over the codebase..
    const Vec2<f32> player_pos = {p.x, p.y};
    const auto top_left = player_pos + slots.offset_top_left;
    draw_point(r, {top_left, g, {0, 255, 0, 255}});

    const auto top_right = player_pos + slots.offset_top_right;
    draw_point(r, {top_right, g, {0, 0, 255, 255}});

    const auto bottom_left = player_pos + slots.offset_bottom_left;
    draw_point(r, {bottom_left, g, {5, 5, 5, 255}});

    const auto bottom_right = player_pos + slots.offset_bottom_right;
    draw_point(r, {bottom_right, g, {125, 125, 125, 255}});
}

void player_draw(SDL_Renderer* r, const Entity& p, Game& g) {
    assert(p.type == Entity_Type::Player);

    entity_draw(r, p, &g);
    if (settings.show_attack_slots) slots_draw(r, p, g);
    if (p.extra_player.has_knife) entity_draw_knife(r, p, &g);
}

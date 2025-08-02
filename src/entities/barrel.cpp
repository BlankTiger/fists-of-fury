#include <cassert>

#include "barrel.h"
#include "../draw.h"
#include "../settings.h"
#include "../utils.h"
#include "../game.h"

Entity barrel_init(Game& g, Barrel_Init_Opts opts) {
    const f32 barrel_w = 32;
    const f32 barrel_h = 32;
    Entity barrel{};
    barrel.handle                = game_generate_entity_handle(g);
    barrel.type                  = Entity_Type::Barrel;
    barrel.x                     = opts.x;
    barrel.y                     = opts.y;
    barrel.speed                 = 0.05f;
    barrel.health                = opts.health;
    barrel.sprite_frame_w        = barrel_w;
    barrel.sprite_frame_h        = barrel_h;
    barrel.hitbox_offsets        = {-barrel_w/4.5f, -20, barrel_w*2/4.5f, 13};
    barrel.collision_box_offsets = {-barrel_w/4.5f, -6, barrel_w*2/4.5f, 4};
    barrel.shadow_offsets        = {-barrel_w/5, -2, barrel_w*2/5, 3};
    barrel.extra_barrel.state    = Barrel_State::Idle;
    barrel.anim.sprite           = opts.sprite;
    animation_start(barrel.anim, { .anim_idx = (u32)Barrel_Anim::Idle });
    return barrel;
}

static bool handle_knockback(Entity& e, u64 dt) {
    e.x += e.x_vel * dt;
    e.z_vel += settings.gravity * dt;
    e.z += e.z_vel * dt;

    if (e.z >= settings.ground_level) {
        e.z = settings.ground_level;
        e.z_vel = 0.0f;
    }

    return e.z_vel == 0.0f && e.x_vel == 0.0f;
}

Update_Result barrel_update(Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Barrel);

    animation_update(e.anim, g.dt, g.dt_real);

    switch (e.extra_barrel.state) {
        case (Barrel_State::Idle): {
            while (!e.damage_queue.empty()) {
                const auto dmg = e.damage_queue.back();
                e.health -= dmg.amount;
                e.damage_queue.pop_back();
                if (e.health <= 0) {
                    e.extra_barrel.state = Barrel_State::Destroyed;
                    if (dmg.going_to == Direction::Left) {
                        e.x_vel = -settings.barrel_knockback_velocity;
                    }
                    else if (dmg.going_to == Direction::Right) {
                        e.x_vel = settings.barrel_knockback_velocity;
                    }
                    else {
                        unreachable("shouldnt ever get a different direction");
                    }
                    e.z_vel = settings.barrel_jump_velocity;
                    animation_start(e.anim, {
                        .anim_idx = (u32)Barrel_Anim::Destroyed,
                        .fadeout = { .enabled = true, .perc_per_sec = 1.9f }
                    });
                }
            }
            return Update_Result::None;
            break;
        }

        case (Barrel_State::Destroyed): {
            auto finished = handle_knockback(e, g.dt);
            if (finished && animation_is_finished(e.anim)) {
                return Update_Result::Remove_Me;
            }
            break;
        }
    }

    return Update_Result::None;
}

void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    entity_draw(r, e, &g);
}

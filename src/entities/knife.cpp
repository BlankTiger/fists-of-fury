#include <cassert>
#include <span>

#include "knife.h"
#include "../utils.h"

Entity knife_init(Game& g, Knife_Init_Opts opts) {
    const auto sprite_frame_w = 48;
    const auto sprite_frame_h = 48;
    Entity knife = {};
    knife.handle = game_generate_entity_handle(g);
    knife.speed = 0.08f;
    knife.type = Entity_Type::Knife;
    knife.x = opts.position.x;
    knife.y = opts.position.y;
    knife.dir = opts.dir;
    if (knife.dir == Direction::Right) {
        knife.x += 10.0f;
    }
    else if (knife.dir == Direction::Left) {
        knife.x -= 10.0f;
    }

    knife.sprite_frame_w = sprite_frame_w;
    knife.sprite_frame_h = sprite_frame_h;

    knife.extra_knife.state = opts.state;
    knife.extra_knife.thrown_by = opts.thrown_by;

    knife.hurtbox_offsets       = {-4.8f, -sprite_frame_h/2.8f, sprite_frame_w/4.5f, sprite_frame_h/7.0f};
    switch (knife.extra_knife.state) {
        case Knife_State::Thrown: {
            // collision offsets are off to the left, because its easier to handle cleanup when knife goes off screen
            // and we dont collide the knife with anything when its flying anyway
            knife.collision_box_offsets = {-18.0f, -sprite_frame_h/3.2f, 1.0f, 1.0f};
            if (knife.dir == Direction::Left) {
                knife.collision_box_offsets.x = -knife.collision_box_offsets.x;
            }
        } break;

        case Knife_State::Falling: {
            knife.collision_box_offsets = knife.hurtbox_offsets;
        } break;
    }

    knife.anim.sprite = &g.sprite_knife;

    Anim_Start_Opts anim_opts = {};
    anim_opts.looping = true;
    switch (knife.extra_knife.state) {
        case Knife_State::Thrown: {
            anim_opts.anim_idx = (u32)Knife_Anim::Thrown;
        } break;

        case Knife_State::Falling: {
            anim_opts.anim_idx = (u32)Knife_Anim::Falling;
        } break;
    }
    animation_start(knife.anim, anim_opts);

    return knife;
}

static bool handle_dealing_damage(const Entity& e, Game& g) {
    auto hurtbox = entity_get_world_hurtbox(e);
    bool hit_something = false;

    for (auto& other_e : g.entities) {
        if (other_e.type == Entity_Type::Knife
            || other_e.type == Entity_Type::Barrel
            || other_e.type == e.extra_knife.thrown_by
        ) continue;

        auto hitbox = entity_get_world_hitbox(other_e);
        if (SDL_HasRectIntersectionFloat(&hurtbox, &hitbox)) {
            hit_something = true;
            other_e.damage_queue.push_back({settings.knife_damage, e.dir, Hit_Type::Normal});
            break;
        }
    }

    return hit_something;
}

static bool handle_movement(Entity& e, const Game& g) {
    switch (e.extra_knife.state) {
        case Knife_State::Thrown: {
            if (e.dir == Direction::Right) {
                e.x_vel = settings.knife_velocity;
            }
            else if (e.dir == Direction::Left) {
                e.x_vel = -settings.knife_velocity;
            }
            else {
                unreachable("shouldnt get any other direction");
            }
        } break;

        case Knife_State::Falling: {

        } break;
    }

    auto in_bounds = entity_movement_handle_collisions_and_pos_change(e, &g, knife_collide_opts);
    return in_bounds;
}

Update_Result knife_update(Entity& e, Game& g) {
    assert(e.type == Entity_Type::Knife);

    auto in_bounds = handle_movement(e, g);
    if (!in_bounds) {
        return Update_Result::Remove_Me;
    }

    auto hit_something = handle_dealing_damage(e, g);
    if (hit_something) {
        return Update_Result::Remove_Me;
    }

    animation_update(e.anim);
    return Update_Result::None;
}

void knife_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Knife);

    entity_draw(r, e, &g);
}

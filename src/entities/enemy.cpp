#include <cassert>

#include "enemy.h"
#include "../draw.h"

Entity enemy_init(const Game& g, Enemy_Init_Opts opts) {
    const auto sprite_frame_h = 48;
    const auto sprite_frame_w = 48;
    Entity enemy{};
    enemy.x                     = opts.x;
    enemy.y                     = opts.y;
    enemy.health                = opts.health;
    enemy.damage                = opts.damage;
    enemy.speed                 = opts.speed;
    enemy.type                  = Entity_Type::Enemy;
    enemy.extra_enemy.type      = opts.type;
    enemy.sprite_frame_w        = sprite_frame_w;
    enemy.sprite_frame_h        = sprite_frame_h;
    enemy.collision_box_offsets = {-sprite_frame_w/7,    -3,  2*sprite_frame_w/7,    4};
    enemy.hurtbox_offsets       = {sprite_frame_w/7,     -16, 10,                    6};
    enemy.hitbox_offsets        = {-sprite_frame_w/6.5f, -23, 2*sprite_frame_w/6.5f, 23};
    enemy.shadow_offsets        = {-7,                   -1,  14,                    2};

    switch (opts.type) {
        case Enemy_Type::Goon: {
            enemy.anim.sprite = &g.sprite_enemy_goon;
            enemy.extra_enemy.state.e = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Thug: {
            enemy.anim.sprite = &g.sprite_enemy_thug;
            enemy.extra_enemy.state.e = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Punk: {
            enemy.anim.sprite = &g.sprite_enemy_punk;
            enemy.extra_enemy.state.e = Enemy_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Anim::Standing, .looping = true });
            break;
        }

        case Enemy_Type::Boss: {
            enemy.anim.sprite = &g.sprite_enemy_boss;
            enemy.extra_enemy.state.e_boss = Enemy_Boss_State::Standing;
            animation_start(enemy.anim, { .anim_idx = (u32)Enemy_Boss_Anim::Standing, .looping = true });
            break;
        }
    }

    return enemy;
}

void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Enemy);

    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(e);
    Vec2<f32> screen_coords = game_get_screen_coords(g, drawing_coords);
    screen_coords.y += e.z; // for jumping

    const SDL_FlipMode flip = (e.dir == Direction::Left) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    bool ok = sprite_draw_at_dst(
        *e.anim.sprite,
        r,
        {
            .x_dst = screen_coords.x,
            .y_dst = screen_coords.y,
            .row   = e.anim.frames.idx,
            .col   = e.anim.frames.frame_current,
            .flip  = flip
        }
    );
    if (!ok) SDL_Log("Failed to draw enemy sprite! SDL err: %s\n", SDL_GetError());

    Vec2<f32> world_coords = {e.x, e.y};
    draw_shadow(r, { .world_coords = world_coords, .shadow_offsets = e.shadow_offsets, .g = g });

    // drawing debug *box
    {
        if (settings.show_collision_boxes) draw_collision_box(r, world_coords, e.collision_box_offsets, g);

        // this is so that both hurtbox and hitbox go along with the player when he jumps
        world_coords.y += e.z;
        if (settings.show_hurtboxes) draw_hurtbox(r, world_coords, e.hurtbox_offsets, g);

        if (settings.show_hitboxes) draw_hitbox(r, world_coords, e.hitbox_offsets, g);
    }
}

Update_Result enemy_update(Entity& e) {
    // TODO: remove this
    e.health = e.health;
    return Update_Result::None;
}


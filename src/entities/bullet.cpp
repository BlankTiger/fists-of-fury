#include "bullet.h"
#include "../game.h"
#include "../draw.h"
#include "../utils.h"

#include <cassert>

static Entity* bullet_find_target_in_path(Entity_Type shot_by, Vec2<f32> pos_start, f32 z, Direction dir, Game& g) {
    SDL_FRect hurtbox = {pos_start.x, pos_start.y + z, SCREEN_WIDTH, 1.0f};
    if (dir == Direction::Right) {
    } else if (dir == Direction::Left) {
        hurtbox.x = pos_start.x - SCREEN_WIDTH;
    } else {
        unreachable("not possible");
    }

    for (auto& entity : g.entities) {
        if (entity.type == shot_by)                  continue;
        if (entity.type == Entity_Type::Collectible) continue;
        if (entity.type == Entity_Type::Barrel)      continue;

        auto hitbox = entity_get_world_hitbox(entity);
        if (SDL_HasRectIntersectionFloat(&hurtbox, &hitbox)) {
            return &entity;
        }
    }

    return nullptr;
}

Entity bullet_init(Game& g, Bullet_Init_Opts opts) {
    Entity bullet = {};

    auto pos_start = opts.pos_creator;
    pos_start.x   += opts.offsets.x;

    bullet.type                   = Entity_Type::Bullet;
    bullet.handle                 = game_generate_entity_handle(g);
    bullet.extra_bullet.pos_start = pos_start;
    bullet.extra_bullet.pos_curr  = pos_start;
    bullet.x                      = pos_start.x;
    bullet.y                      = pos_start.y;
    bullet.z                      = opts.offsets.y;
    bullet.dir                    = opts.dir;

    Entity* target = bullet_find_target_in_path(opts.shot_by, pos_start, bullet.z, opts.dir, g);
    if (target) {
        bullet.extra_bullet.length = std::abs(target->x - bullet.x);
        target->damage_queue.push_back({settings.gun_damage, bullet.dir, Hit_Type::Knockdown});
    } else {
        bullet.extra_bullet.length = opts.length;
    }

    if (opts.dir == Direction::Left) {
        bullet.extra_bullet.pos_end = pos_start + Vec2{-bullet.extra_bullet.length, opts.thickness};
    } else if (opts.dir == Direction::Right) {
        bullet.extra_bullet.pos_end = pos_start + Vec2{bullet.extra_bullet.length, opts.thickness};
    } else {
        unreachable("shouldnt ever happen");
    }

    bullet.extra_bullet.creation_timestamp = g.time_ms;
    auto ms_per_px                         = 2;
    bullet.extra_bullet.time_of_flight_ms  = ms_per_px * bullet.extra_bullet.length;

    g.entities.push_back(bullet);
    return bullet;
}

static void bullet_lerp_curr_pos(Entity& e, u64 time_ms) {
    auto rate = 1.0f - (f64)(e.extra_bullet.creation_timestamp + e.extra_bullet.time_of_flight_ms - time_ms) / (f64) e.extra_bullet.time_of_flight_ms;
    e.extra_bullet.pos_curr.x = e.extra_bullet.pos_start.x + (e.extra_bullet.pos_end.x - e.extra_bullet.pos_start.x) * rate;
}

Update_Result bullet_update(Entity& e, Game& g) {
    assert(e.type == Entity_Type::Bullet);

    if (g.time_ms > e.extra_bullet.creation_timestamp + e.extra_bullet.time_of_flight_ms) {
        return Update_Result::Remove_Me;
    }

    bullet_lerp_curr_pos(e, g.time_ms);

    return Update_Result::None;
}

void bullet_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Bullet);

    Vec2<f32> screen_curr = game_get_screen_coords(g, e.extra_bullet.pos_curr);
    Vec2<f32> screen_end  = game_get_screen_coords(g, e.extra_bullet.pos_end);

    static const SDL_Color white  = {230, 230, 230, 255};
    static const SDL_Color yellow = {230, 230, 0,   255};

    f32 x1 = screen_curr.x < screen_end.x ? screen_curr.x : screen_end.x;
    f32 x2 = screen_curr.x < screen_end.x ? screen_end.x  : screen_curr.x;
    f32 y1 = screen_curr.y + e.z;
    f32 y2 = screen_end.y  + e.z;

    if (e.dir == Direction::Left) {
        draw_gradient_rect_geometry(r, x1, y1, x2, y2, yellow, white,  yellow, white);
    } else {
        draw_gradient_rect_geometry(r, x1, y1, x2, y2, white,  yellow, white,  yellow);
    }
}

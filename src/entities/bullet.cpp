#include "bullet.h"
#include "../game.h"
#include "../draw.h"

#include <cassert>

Entity bullet_init(Game& g, Bullet_Init_Opts opts) {
    Entity bullet = {};
    bullet.type = Entity_Type::Bullet;
    bullet.handle = game_generate_entity_handle(g);
    bullet.extra_bullet.pos_start = opts.pos_start;
    bullet.extra_bullet.pos_curr  = opts.pos_start;
    bullet.extra_bullet.pos_end   = opts.pos_start + Vec2{opts.length, opts.thickness};
    bullet.extra_bullet.length    = opts.length;
    bullet.extra_bullet.creation_timestamp = g.time_ms;
    bullet.extra_bullet.time_of_flight_ms = 200;

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

void bullet_draw(SDL_Renderer* r, const Entity& e, Game& g) {
    assert(e.type == Entity_Type::Bullet);

    SDL_Color yellowish = {50, 50, 0, 255};
    SDL_Color yellow = {200, 200, 0, 255};
    auto& pos_curr = e.extra_bullet.pos_curr;
    auto& pos_end  = e.extra_bullet.pos_end;
    draw_gradient_rect_geometry(g.renderer, pos_curr.x, pos_curr.y, pos_end.x, pos_end.y, yellowish, yellow, yellowish, yellow);
}

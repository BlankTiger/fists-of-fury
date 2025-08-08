#include "bullet.h"
#include "../game.h"
#include "../draw.h"

#include <cassert>

Entity bullet_init(Game& g, Bullet_Init_Opts opts) {
    Entity bullet = {};
    bullet.type = Entity_Type::Bullet;
    bullet.handle = game_generate_entity_handle(g);
    bullet.extra_bullet.pos_start = opts.pos_start;
    bullet.extra_bullet.length_and_thickness = {opts.length, opts.thickness};
    bullet.extra_bullet.creation_timestamp = g.time_ms;
    bullet.extra_bullet.time_of_flight_ms = 200;
    return bullet;
}

Update_Result bullet_update(Entity& e, Game& g) {
    assert(e.type == Entity_Type::Bullet);

    return Update_Result::None;
}

void bullet_draw(SDL_Renderer* r, const Entity& e, Game& g) {
    assert(e.type == Entity_Type::Bullet);

    SDL_Color yellowish = {50, 50, 0, 255};
    SDL_Color yellow = {200, 200, 0, 255};
    auto& pos_start = e.extra_bullet.pos_start;
    auto& l_and_t = e.extra_bullet.length_and_thickness;
    draw_gradient_rect_geometry(g.renderer, pos_start.x, pos_start.y, l_and_t.x, l_and_t.y, yellowish, yellow, yellowish, yellow);
}

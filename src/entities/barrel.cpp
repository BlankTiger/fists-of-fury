#include "barrel.h"

void barrel_update(Entity& e) {
    // TODO: remove
    e.health = e.health;
}

void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    Vec2<f32> screen_coords = game_get_screen_coords(g, {e.x, e.y});

    bool ok = sprite_draw_at_dst(g.sprite_barrel, r, screen_coords.x, screen_coords.y, 0, 0, SDL_FLIP_NONE);
    if (!ok) SDL_Log("Failed to draw barrel sprite! SDL err: %s\n", SDL_GetError());

    draw_shadow(r, screen_coords, e.shadow_offset, g);
    #if SHOW_COLLISION_BOXES
    draw_collision_box(r, screen_coords, e.collision_box_offsets);
    #endif
}

#include "barrel.h"
#include "../draw.h"
#include "../settings.h"

void barrel_update(Entity& e) {
    // TODO: remove
    e.health = e.health;
}

void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(e);
    const Vec2<f32> screen_coords = game_get_screen_coords(g, drawing_coords);

    bool ok = sprite_draw_at_dst(g.sprite_barrel, r, screen_coords.x, screen_coords.y, 0, 0, SDL_FLIP_NONE);
    if (!ok) SDL_Log("Failed to draw barrel sprite! SDL err: %s\n", SDL_GetError());

    const Vec2<f32> world_coords = {e.x, e.y};
    draw_shadow(r, world_coords, e.shadow_offsets, g);
    #if SHOW_COLLISION_BOXES
    draw_collision_box(r, world_coords, e.collision_box_offsets, g);
    #endif
}

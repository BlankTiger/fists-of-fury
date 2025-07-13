#include <cassert>

#include "barrel.h"
#include "../draw.h"
#include "../settings.h"

Entity barrel_init() {
    const f32 barrel_w = 32;
    const f32 barrel_h = 32;
    Entity barrel{};
    barrel.type                  = Entity_Type::Barrel;
    barrel.x                     = SCREEN_WIDTH / 2;
    barrel.y                     = 45;
    barrel.sprite_frame_w        = barrel_w;
    barrel.sprite_frame_h        = barrel_h;
    barrel.hitbox_offsets        = {-barrel_w/4.5f, -20, barrel_w*2/4.5f, 13};
    barrel.collision_box_offsets = {-barrel_w/4.5f, -6, barrel_w*2/4.5f, 4};
    barrel.shadow_offsets        = {-barrel_w/5, -2, barrel_w*2/5, 3};
    return barrel;
}

void barrel_update(Entity& e) {
    assert(e.type == Entity_Type::Barrel);

    // TODO: remove
    e.health = e.health;
}

void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type == Entity_Type::Barrel);

    const Vec2<f32> drawing_coords = entity_offset_to_bottom_center(e);
    const Vec2<f32> screen_coords = game_get_screen_coords(g, drawing_coords);

    bool ok = sprite_draw_at_dst(g.sprite_barrel, r, screen_coords.x, screen_coords.y, 0, 0, SDL_FLIP_NONE);
    if (!ok) SDL_Log("Failed to draw barrel sprite! SDL err: %s\n", SDL_GetError());

    const Vec2<f32> world_coords = {e.x, e.y};
    draw_shadow(r, world_coords, e.shadow_offsets, g);

    {
        #if SHOW_COLLISION_BOXES || DEV_MODE
        draw_collision_box(r, world_coords, e.collision_box_offsets, g);
        #endif

        #if SHOW_HITBOXES || DEV_MODE
        draw_hitbox(r, world_coords, e.hitbox_offsets, g);
        #endif

        // NOTE: barrels dont have hurtboxes obviously
    }
}

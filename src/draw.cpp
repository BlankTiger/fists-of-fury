#include "draw.h"
#include "settings.h"

internal void _draw_box(SDL_Renderer* r, const SDL_FRect& box, const f32 colors[4]) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, colors[0], colors[1], colors[2], colors[3]);
    bool ok = SDL_RenderRect(r, &box);
    if (!ok) SDL_Log("Failed to draw box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, colors[0] / 2, colors[1] / 2, colors[2] / 2, colors[3] / 2);
    ok = SDL_RenderFillRect(r, &box);
    if (!ok) SDL_Log("Failed to draw box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void draw_collision_box(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& offsets, const Game& g) {
    const SDL_FRect collision_box_screen = {
        (world_coords.x + offsets.x) - g.camera.x,
        (world_coords.y + offsets.y) - g.camera.y,
        offsets.w,
        offsets.h
    };
    _draw_box(r, collision_box_screen, COLLISION_BOX_COLORS);
}

void draw_hurtbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hurtbox_offsets, const Game& g) {
    const SDL_FRect hurtbox_screen = {
        (world_coords.x + hurtbox_offsets.x) - g.camera.x,
        (world_coords.y + hurtbox_offsets.y) - g.camera.y,
        hurtbox_offsets.w,
        hurtbox_offsets.h
    };
    _draw_box(r, hurtbox_screen, HURTBOX_COLORS);
}

void draw_hitbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hitbox_offsets, const Game& g) {
    const SDL_FRect hitbox_screen = {
        (world_coords.x + hitbox_offsets.x) - g.camera.x,
        (world_coords.y + hitbox_offsets.y) - g.camera.y,
        hitbox_offsets.w,
        hitbox_offsets.h
    };
    _draw_box(r, hitbox_screen, HITBOX_COLORS);
}

void draw_level(SDL_Renderer* r, const Game& g) {
    const SDL_FRect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderTexture(r, g.bg.img, &g.camera, &dst);
    #if SHOW_COLLISION_BOXES
    for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
        // Draw collision boxes relative to camera
        SDL_FRect screen_box = {
            box.x - g.camera.x,
            box.y - g.camera.y,
            box.w,
            box.h
        };
        _draw_box(r, screen_box, COLLISION_BOX_COLORS);
    }
    #endif
}

void draw_shadow(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& offsets, const Game& g) {
    const SDL_FRect shadow_box_screen = {
        (world_coords.x + offsets.x) - g.camera.x,
        (world_coords.y + offsets.y) - g.camera.y,
        offsets.w,
        offsets.h
    };
    bool ok = SDL_RenderTexture(r, g.entity_shadow.img, NULL, &shadow_box_screen);
    if (!ok) SDL_Log("Failed to draw shadow! SDL err: %s\n", SDL_GetError());
}

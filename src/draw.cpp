#include "draw.h"
#include "settings.h"

internal void _draw_collision_box(SDL_Renderer* r, const SDL_FRect& box) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 255, 2, 0, 200);
    bool ok = SDL_RenderRect(r, &box);
    if (!ok) SDL_Log("Failed to draw background collision box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 150, 0, 0, 100);
    ok = SDL_RenderFillRect(r, &box);
    if (!ok) SDL_Log("Failed to draw background collision box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawColor(r, 0, 0, 0, SDL_ALPHA_OPAQUE);
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
        _draw_collision_box(r, screen_box);
    }
    #endif
}

void draw_collision_box(SDL_Renderer* r, const Vec2<f32>& screen_coords, const SDL_FRect& offsets) {
    const SDL_FRect collision_box = {
        offsets.x + screen_coords.x,
        offsets.y + screen_coords.y,
        offsets.w,
        offsets.h
    };
    _draw_collision_box(r, collision_box);
}

void draw_shadow(SDL_Renderer* r, const Vec2<f32>& screen_coords, const SDL_FRect& offsets, const Game& g) {
    const SDL_FRect shadow_screen = {
        offsets.x + screen_coords.x,
        offsets.y + screen_coords.y,
        offsets.w,
        offsets.h
    };
    bool ok = SDL_RenderTexture(r, g.entity_shadow.img, NULL, &shadow_screen);
    if (!ok) SDL_Log("Failed to draw shadow! SDL err: %s\n", SDL_GetError());
}

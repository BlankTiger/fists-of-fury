#include "draw.h"
#include "settings.h"

static void _draw_box(SDL_Renderer* r, const SDL_FRect& box, const std::array<f32, 4> colors_border, const std::array<f32, 4> colors_fill) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, colors_border[0], colors_border[1], colors_border[2], colors_border[3]);
    bool ok = SDL_RenderRect(r, &box);
    if (!ok) SDL_Log("Failed to draw box! SDL err: %s\n", SDL_GetError());

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, colors_fill[0], colors_fill[1], colors_fill[2], colors_fill[3]);
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
    _draw_box(r, collision_box_screen, settings.colors_collision_box_border, settings.colors_collision_box_fill);
}

void draw_hurtbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hurtbox_offsets, const Game& g) {
    const SDL_FRect hurtbox_screen = {
        (world_coords.x + hurtbox_offsets.x) - g.camera.x,
        (world_coords.y + hurtbox_offsets.y) - g.camera.y,
        hurtbox_offsets.w,
        hurtbox_offsets.h
    };
    _draw_box(r, hurtbox_screen, settings.colors_hurtbox_border, settings.colors_hurtbox_fill);
}

void draw_hitbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hitbox_offsets, const Game& g) {
    const SDL_FRect hitbox_screen = {
        (world_coords.x + hitbox_offsets.x) - g.camera.x,
        (world_coords.y + hitbox_offsets.y) - g.camera.y,
        hitbox_offsets.w,
        hitbox_offsets.h
    };
    _draw_box(r, hitbox_screen, settings.colors_hitbox_border, settings.colors_hitbox_fill);
}

void draw_level(SDL_Renderer* r, const Game& g) {
    const SDL_FRect dst = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderTexture(r, g.bg.img, &g.camera, &dst);
    if (settings.show_collision_boxes) {
        for (const auto& box : level_info_get_collision_boxes(g.curr_level_info)) {
            // Draw collision boxes relative to camera
            SDL_FRect screen_box = {
                box.x - g.camera.x,
                box.y - g.camera.y,
                box.w,
                box.h
            };
            _draw_box(r, screen_box, settings.colors_collision_box_border, settings.colors_collision_box_fill);
        }
    }
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

void draw_box(SDL_Renderer* r, const SDL_FRect dst, const Draw_Box_Opts& opts) {
    _draw_box(r, dst, opts.colors_border, opts.colors_fill);
}

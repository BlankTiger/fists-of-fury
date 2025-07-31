#pragma once

#include <array>

#include <SDL3/SDL.h>

#include "game.h"
#include "vec2.h"

void draw_level(SDL_Renderer* r, const Game& g);

struct Draw_Shadow_Opts {
    const Vec2<f32>& world_coords;
    const SDL_FRect& shadow_offsets;
    const Game&      g;
    const f32        opacity = 1.0f;
};

void draw_shadow(SDL_Renderer* r, Draw_Shadow_Opts opts);
void draw_collision_box(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& collision_box_offsets, const Game& g);
void draw_hurtbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hurtbox_offsets, const Game& g);
void draw_hitbox(SDL_Renderer* r, const Vec2<f32>& world_coords, const SDL_FRect& hitbox_offsets, const Game& g);

using Color = std::array<f32, 4>;

struct Draw_Box_Opts {
    Color colors_border;
    Color colors_fill;
};

void _draw_box(SDL_Renderer* r, const SDL_FRect& box, const std::array<f32, 4> colors_border, const std::array<f32, 4> colors_fill);
void draw_box(SDL_Renderer* r, const SDL_FRect dst, Draw_Box_Opts opts);

struct Draw_Text_Opts {
    Color color;
};

void draw_text(SDL_Renderer* r, const SDL_FRect dst, Draw_Text_Opts opts);

struct Draw_Point_Opts {
    Vec2<f32>   dst_world_coords; 
    const Game& g;
    Color       color = {255, 0, 0, 255};
};

void draw_point(SDL_Renderer* r, Draw_Point_Opts opts);

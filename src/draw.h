#pragma once

#include <SDL3/SDL.h>
#include "game.h"
#include "vec2.h"

void draw_level(SDL_Renderer* r, const Game& g);
void draw_shadow(SDL_Renderer* r, const Vec2<f32>& coords, const SDL_FRect& shadow_offsets, const Game& g);
void draw_collision_box(SDL_Renderer* r, const Vec2<f32>& coords, const SDL_FRect& collision_box_offsets);

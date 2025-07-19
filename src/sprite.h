#pragma once

#include <span>
#include <SDL3/SDL.h>

#include "number_types.h"

struct Img {
    SDL_Texture* img;
    f32          width;
    f32          height;
};

// Has to be called after initializing the renderer.
//
// returns false on error
bool img_load(Img& i, SDL_Renderer* r, const char* path);

struct Sprite {
    Img                  img;
    u32                  max_frames_in_row_count;
    std::span<const u32> frames_in_each_row;
};

// Asserts that `max_frames_in_row_count` and `frames_in_each_row`
// are already initialized.
bool sprite_load(Sprite& s, SDL_Renderer* r, const char* path);

struct Sprite_Draw_Opts {
    f32          x_dst;
    f32          y_dst;
    u64          row;
    u64          col;
    SDL_FlipMode flip    = SDL_FLIP_NONE;
    f32          opacity = 1.0f;
};

bool sprite_draw_at_dst(const Sprite& s, SDL_Renderer* r, Sprite_Draw_Opts opts);

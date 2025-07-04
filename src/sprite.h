#pragma once

#include <span>

#include "number_types.h"
#include "array_view.h"

// forward declaration to omit including SDL3
struct SDL_Texture;
struct SDL_Renderer;
struct SDL_FRect;

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

struct Src_Rect_With_Size {
    SDL_FRect src;
    f32       width;
    f32       height;
};

Src_Rect_With_Size sprite_get_src_rect(const Sprite& s, u32 row, u32 col);

bool sprite_draw_at_dst(const Sprite& s, SDL_Renderer* r, f32 x_dst, f32 y_dst, u32 row, u32 col);

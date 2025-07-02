#pragma once

#include "number_types.h"
#include "array_view.h"

struct SDL_Texture;

struct Img {
    SDL_Texture* img;
    f32          width;
    f32          height;
};

struct Sprite {
    Img             img;
    u32             frame_width;
    u32             frame_height;
    Array_View<u32> frames_in_each_row;
};


struct SDL_Renderer;

// Has to be called after initializing the renderer.
//
// returns false on error
bool img_load(Img& i, SDL_Renderer* r, const char* path);

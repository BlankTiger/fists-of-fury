#ifndef SPRITE_H
#define SPRITE_H

#include "number_types.h"
#include "array_view.h"

struct Img {
    SDL_Texture* img;
    f32          width;
    f32          height;
};

struct Sprite {
    Img              img;
    u32              frame_width;
    u32              frame_height;
    Array_View<u32>  frames_in_each_row;
};

#endif // SPRITE_H

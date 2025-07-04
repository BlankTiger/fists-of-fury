#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL.h>
#include "sprite.h"
#include <cassert>

bool img_load(Img& i, SDL_Renderer* r, const char* path) {
    i.img = IMG_LoadTexture(r, path);
    if (i.img == nullptr) {
        SDL_Log("Could not load img! SDL err: %s\n", SDL_GetError());
        return false;
    }

    bool ok = SDL_GetTextureSize(i.img, &i.width, &i.height);
    if (!ok) {
        SDL_Log("Could not load img size! SDL err: %s\n", SDL_GetError());
        return false;
    }

    ok = SDL_SetTextureScaleMode(i.img, SDL_SCALEMODE_NEAREST);
    if (!ok) {
        SDL_Log("Could not set texture scaling to nearest neighbor! SDL err: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool sprite_load(Sprite& s, SDL_Renderer* r, const char* path) {
    assert(s.max_frames_in_row_count   > 0);
    assert(s.frames_in_each_row.size() > 0);

    return img_load(s.img, r, path);
}

Src_Rect_With_Size sprite_get_src_rect(const Sprite& s, u32 row, u32 col) {
    assert(row < s.frames_in_each_row.size());
    assert(col < s.frames_in_each_row[row]);
    f32 x = 5;
    f32 y = 5;
    f32 width = 10;
    f32 height = 10;
    return {
        .src    = { x, y, width, height },
        .width  = 8.,
        .height = 8.,
    };
}

bool sprite_draw_at_dst(const Sprite& s, SDL_Renderer* r, f32 x_dst, f32 y_dst, u64 row, u64 col) {
    assert(row < s.frames_in_each_row.size());
    assert(col < s.frames_in_each_row[row]);

    const f32 width = s.img.width / s.max_frames_in_row_count;
    const f32 height = s.img.height / s.frames_in_each_row.size();
    const f32 x = col * width;
    const f32 y = row * height;
    const SDL_FRect src = {x, y, width, height};
    const SDL_FRect dst = {x_dst, y_dst, width, height};
    return SDL_RenderTexture(r, s.img.img, &src, &dst);
}

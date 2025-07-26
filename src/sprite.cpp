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

static bool sprite_range_check(const Sprite& s, const Sprite_Draw_Opts& opts) {
    return opts.row < s.frames_in_each_row.size()
        && opts.col < s.frames_in_each_row[opts.row];
}

// provided x_dst and y_dst must be in screen coordinates (in other words relative to the camera), not world coordinates
bool sprite_draw_at_dst(const Sprite& s, SDL_Renderer* r, Sprite_Draw_Opts opts) {
    if (!opts.return_on_failed_range_checks) {
        assert(sprite_range_check(s, opts));
    } else {
        if (!sprite_range_check(s, opts)) return false;
    }

    const f32 width = s.img.width / s.max_frames_in_row_count;
    const f32 height = s.img.height / s.frames_in_each_row.size();
    const f32 x = opts.col * width;
    const f32 y = opts.row * height;
    const SDL_FRect src = {x, y, width, height};
    const SDL_FRect dst = {opts.x_dst, opts.y_dst, width, height};

    bool ok = true;
    if (opts.opacity < 1.0f) {
        ok = SDL_SetTextureBlendMode(s.img.img, SDL_BLENDMODE_BLEND);
        if (!ok) SDL_Log("Failed to draw sprite! SDL err: %s\n", SDL_GetError());
        ok = SDL_SetTextureAlphaModFloat(s.img.img, opts.opacity);
        if (!ok) SDL_Log("Failed to draw sprite! SDL err: %s\n", SDL_GetError());
    }
    ok = SDL_RenderTextureRotated(r, s.img.img, &src, &dst, 0., NULL, opts.flip);
    if (!ok) SDL_Log("Failed to draw sprite! SDL err: %s\n", SDL_GetError());
    ok = SDL_SetTextureAlphaModFloat(s.img.img, 1.0f);
    if (!ok) SDL_Log("Failed to change opacity for drawn sprite! SDL err: %s\n", SDL_GetError());

    return ok;
}

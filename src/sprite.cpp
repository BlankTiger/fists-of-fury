#include <SDL3_image/SDL_image.h>
#include "sprite.h"

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


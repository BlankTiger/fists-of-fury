#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

int SCREEN_WIDTH  = 640;
int SCREEN_HEIGHT = 480;

static SDL_Window*  window  = nullptr;
static SDL_Surface* surface = nullptr;

bool init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL err: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Fists of Fury", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL err: %s\n", SDL_GetError());
        return false;
    }

    surface = SDL_GetWindowSurface(window);
    return true;
}

void close() {
    SDL_DestroyWindow(window);
    window = nullptr;
    surface = nullptr;
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!init()) {
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }

        SDL_FillSurfaceRect(
            surface,
            nullptr,
            SDL_MapSurfaceRGB(surface, 0xFF, 0xFF, 0xFF)
        );

        SDL_UpdateWindowSurface(window);
    }

    close();
    return 0;
}

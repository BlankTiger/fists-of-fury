#include "enemy.h"
#include "../draw.h"

void enemy_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    // Draw entity relative to camera position
    f32 screen_x = e.x - g.camera.x;
    f32 screen_y = e.y - g.camera.y;

    const SDL_FRect dst = {screen_x, screen_y, g.entity_shadow.width, g.entity_shadow.height};
    SDL_RenderTexture(r, g.entity_shadow.img, NULL, &dst);
    #if SHOW_COLLISION_BOXES
    // TODO: Add collision box drawing with camera offset
    #endif
}

void enemy_update(Entity& e) {
    // TODO: remove this
    e.health = e.health;
}


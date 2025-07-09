#include "barrel.h"

void barrel_update(Entity& e) {
    // TODO: remove
    e.health = e.health;
}

void barrel_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    bool ok = sprite_draw_at_dst(g.sprite_barrel, r, e.x, e.y, 0, 0, SDL_FLIP_NONE);
    // TODO:
    if (!ok) {}
}

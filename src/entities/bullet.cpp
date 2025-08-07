#include "bullet.h"
#include "../game.h"

#include <cassert>

Entity bullet_init(Game& g, Bullet_Init_Opts opts) {
    Entity bullet = {};
    bullet.handle = game_generate_entity_handle(g);
    return bullet;
}

Update_Result bullet_update(Entity& e, Game& g) {
    assert(e.type == Entity_Type::Bullet);

    return Update_Result::None;
}

void bullet_draw(SDL_Renderer* r, const Entity& e, Game& g) {}

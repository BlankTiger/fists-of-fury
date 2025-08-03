#include <cassert>
#include <span>

#include "gun.h"
#include "../utils.h"

Entity gun_init(Game& g, Knife_Init_Opts opts) {
    const auto sprite_frame_w = 48;
    const auto sprite_frame_h = 48;
    Entity knife                 = {};
    knife.handle                 = game_generate_entity_handle(g);
    knife.type                   = Entity_Type::Collectible;
    knife.extra_collectible.type = Collectible_Type::Gun;
    knife.sprite_frame_w = sprite_frame_w;
    knife.sprite_frame_h = sprite_frame_h;
    return knife;
}

Update_Result gun_update(Entity& e, Game& g) {
    assert(e.type                   == Entity_Type::Collectible);
    assert(e.extra_collectible.type == Collectible_Type::Gun);

    switch (e.extra_collectible.gun.state) {
        case Gun_State::Dropped: {
        } break;

        case Knife_State::On_The_Ground: {
        } break;

        case Knife_State::Picked_Up: {
            return Update_Result::Remove_Me;
        } break;
    }

    animation_update(e.anim, g.dt, g.dt_real);
    return Update_Result::None;
}

void gun_draw(SDL_Renderer* r, const Entity& e, const Game& g) {
    assert(e.type                   == Entity_Type::Collectible);
    assert(e.extra_collectible.type == Collectible_Type::Gun);

    entity_draw(r, e, &g);
}

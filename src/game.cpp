#include "game.h"

Vec2<f32> game_get_screen_coords(const Game& g, Vec2<f32> world_coords) {
    return {
        .x = world_coords.x - g.camera.x,
        .y = world_coords.y - g.camera.y,
    };
}

Entity game_get_player(const Game& g) {
    return g.entities[g.idx_player];
}

Entity& game_get_player_mutable(Game& g) {
    return g.entities[g.idx_player];
}

Handle game_generate_entity_handle(Game& g) {
    Handle h = { .id = g.last_entity_id + 1 };
    g.last_entity_id++;
    return h;
}

Entity* game_get_mutable_entity_by_handle(Game& g, const Handle& h) {
    for (auto& entity : g.entities) {
        if (entity.handle == h) return &entity;
    }

    return NULL;
}

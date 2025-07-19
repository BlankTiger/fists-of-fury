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

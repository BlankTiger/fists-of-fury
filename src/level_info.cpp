#include "level_info.h"

std::span<const SDL_FRect> level_info_get_collision_boxes(const Level_Info& li) {
    return std::span(li.collision_boxes);
}

SDL_FRect level_info_get_collision_box(const Level_Info& li, Border border) {
    return li.collision_boxes[(usize)border];
}

void level_info_update_collision_box(Level_Info& info, Border border, SDL_FRect box) {
    info.collision_boxes[(usize)border] = box;
}

Level_Info level_data_get_level(Level level) {
    return level_data[(usize)level];
}

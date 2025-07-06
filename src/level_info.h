#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <initializer_list>
#include <span>

#include "number_types.h"

enum struct Level : usize {
    Street,
    Bar,
    Count // KEEP THIS LAST
};
static_assert((usize)Level::Street == 0);
static_assert((usize)Level::Bar    == 1);

enum struct Border : usize {
    Top,
    Bottom,
    Left,
    Right
};

struct Level_Info {
    std::array<SDL_FRect, 4> collision_boxes;
    const char*              bg_path;
};

std::span<const SDL_FRect> level_info_get_collision_boxes(const Level_Info& li) {
    return std::span(li.collision_boxes);
}

SDL_FRect level_info_get_collision_box(const Level_Info& li, Border border) {
    return li.collision_boxes[(usize)border];
}

void level_info_update_collision_box(Level_Info& info, Border border, SDL_FRect box) {
    info.collision_boxes[(usize)border] = box;
}

constexpr std::array<Level_Info, (usize)Level::Count> level_data = []() {
    std::array<Level_Info, (usize)Level::Count> data{};

    data[(usize)Level::Street] = {};
    data[(usize)Level::Street].bg_path  = "assets/art/backgrounds/street-background.png";
    data[(usize)Level::Street].collision_boxes[(usize)Border::Top]    = {0,   0,  150, 30};
    data[(usize)Level::Street].collision_boxes[(usize)Border::Bottom] = {0,   64, 150, 10};
    data[(usize)Level::Street].collision_boxes[(usize)Border::Left]   = {-1,  0,  1,   64};
    data[(usize)Level::Street].collision_boxes[(usize)Border::Right]  = {400, 0,  1,   64};

    data[(usize)Level::Bar] = {};
    data[(usize)Level::Bar].bg_path  = "assets/art/backgrounds/bar-background.png";
    data[(usize)Level::Bar].collision_boxes[(usize)Border::Top]    = {0,   0,  150, 32};
    data[(usize)Level::Bar].collision_boxes[(usize)Border::Bottom] = {0,   64, 150, 10};
    data[(usize)Level::Bar].collision_boxes[(usize)Border::Left]   = {-1,  0,  1,   64};
    data[(usize)Level::Bar].collision_boxes[(usize)Border::Right]  = {400, 0,  1,   64};

    return data;
}();

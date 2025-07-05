#pragma once

#include "number_types.h"
#include <array>
#include <initializer_list>
#include <span>

enum struct Level : usize {
    Street,
    Bar,
    Count // KEEP THIS LAST
};

static_assert((usize)Level::Street == 0);
static_assert((usize)Level::Bar    == 1);

struct Box {
    f32 x, y, width, height;
};

struct Level_Info {
    const char*         bg_path;
    std::array<Box, 10> collision_boxes;
    usize               box_count;
};

std::span<const Box> level_info_boxes(const Level_Info& li) {
    return std::span(li.collision_boxes.data(), li.box_count);
}

constexpr Level_Info make_level_info(const char* path, std::initializer_list<Box> boxes) {
    Level_Info info{};
    info.bg_path = path;
    info.box_count = boxes.size();

    usize i = 0;
    for (const auto& box : boxes) {
        info.collision_boxes[i++] = box;
    }

    return info;
}

constexpr std::array<Level_Info, (usize)Level::Count> level_data = []() {
    std::array<Level_Info, (usize)Level::Count> data{};

    data[(usize)Level::Street] = make_level_info(
        "assets/art/backgrounds/street-background.png",
        {
            {10, 20, 50, 30},
            {100, 150, 40, 60}
        }
    );

    data[(usize)Level::Bar] = make_level_info(
        "assets/art/backgrounds/bar-background.png",
        {
            {10, 20, 50, 30},
            {100, 150, 40, 60}
        }
    );

    return data;
}();

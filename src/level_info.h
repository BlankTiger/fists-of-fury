#pragma once

#include "number_types.h"
#include <array>
#include <initializer_list>
#include <span>

enum struct Level : usize {
    Street,
    Count // KEEP THIS LAST
};

struct Box {
    f32 x, y, width, height;
};

struct Level_Info {
    std::array<Box, 10> collision_boxes;
    usize box_count;
};

std::span<const Box> level_info_boxes(const Level_Info& li) {
    return std::span(li.collision_boxes.data(), li.box_count);
}

constexpr Level_Info make_level_info(std::initializer_list<Box> boxes) {
    Level_Info info{};
    info.box_count = boxes.size();

    usize i = 0;
    for (const auto& box : boxes) {
        info.collision_boxes[i++] = box;
    }

    return info;
}

constexpr std::array<Level_Info, (usize)Level::Count> level_data = []() {
    std::array<Level_Info, (usize)Level::Count> data{};

    data[(usize)Level::Street] = make_level_info({
        {10, 20, 50, 30},
        {100, 150, 40, 60}
    });

    return data;
}();

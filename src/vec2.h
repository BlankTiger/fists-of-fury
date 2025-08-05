#pragma once

#include <cmath>
#include "number_types.h"

template<typename T>
struct Vec2 {
    T x, y;

    Vec2<T> operator+(const Vec2<T>& other) const {
        return {x + other.x, y + other.y};
    }

    Vec2<T>& operator+=(const Vec2<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2<T> operator-(const Vec2<T>& other) const {
        return {x - other.x, y - other.y};
    }

    Vec2<T>& operator-=(const Vec2<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    void normalize() {
        const auto len = std::sqrt(x * x + y * y);
        x /= len;
        y /= len;
    }

    T len() {
        return std::sqrt(x * x + y * y);
    }

    bool within_len_from(const Vec2<T> from, const T len) const {
        return (from - *this).len() < len;
    }
};

extern template struct Vec2<f32>;

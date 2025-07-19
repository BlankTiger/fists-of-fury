#pragma once

#include <cmath>

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

    f32 len() {
        return std::sqrt(x * x + y * y);
    }
};

template struct Vec2<f32>;

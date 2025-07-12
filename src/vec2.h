#pragma once

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
};

template struct Vec2<f32>;

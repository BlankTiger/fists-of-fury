#pragma once

#include <array>
#include "number_types.h"

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;

struct Settings {
    bool show_collision_boxes  = false;
    bool show_hurtboxes        = false;
    bool show_hitboxes         = false;
    bool show_attack_slots     = false;
    bool dev_mode              = true;

    std::array<f32, 4> colors_collision_box_border = {255,      2,        0,        200};
    std::array<f32, 4> colors_collision_box_fill   = {255/2.0f, 2/2.0f,   0/2.0f,   200/2.0f};

    std::array<f32, 4> colors_hurtbox_border       = {2,        255,      0,        200};
    std::array<f32, 4> colors_hurtbox_fill         = {2/2.0f,   255/2.0f, 0/2.0f,   200/2.0f};

    std::array<f32, 4> colors_hitbox_border        = {0,        2,        255,      200};
    std::array<f32, 4> colors_hitbox_fill          = {0/2.0f,   2/2.0f,   255/2.0f, 200/2.0f};

    std::array<f32, 4> color_text                  = {0,        0,        0,        255};

    f32 font_size_default          = 9.0f;
    f32 fps_max                    = 144.0f;
    f32 gravity                    = 0.00038f;
    f32 jump_velocity              = -0.15f;
    f32 barrel_jump_velocity       = -0.10f;
    f32 barrel_knockback_velocity  = 0.05f;
    f32 enemy_knockback_velocity   = 0.04f;
    f32 enemy_knockdown_velocity   = 0.1f;
    f32 enemy_flying_back_velocity = 0.1f;
    f32 enemy_friction             = 0.003f;
    f32 ground_level               = 0.0f;
};

extern Settings settings;

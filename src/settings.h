#pragma once

#define internal static

#define SHOW_COLLISION_BOXES 1
#define SHOW_HURTBOXES       1
#define SHOW_HITBOXES        1
#define DEV_MODE             1

const f32 COLLISION_BOX_COLORS[] = {255, 2, 0, 200};
const f32 HURTBOX_COLORS[]       = {2, 255, 0, 200};
const f32 HITBOX_COLORS[]        = {0, 2, 255, 200};

const int SCREEN_WIDTH  = 100;
const int SCREEN_HEIGHT = 64;
const int WINDOW_WIDTH  = 1000;
const int WINDOW_HEIGHT = 640;
const f32 FPS_MAX       = 144.0;

const f32 GRAVITY       = 0.00038f;
const f32 JUMP_VELOCITY = -0.15f;
const f32 GROUND_LEVEL  = 0.0f;

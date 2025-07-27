#pragma once

#include "../number_types.h"
#include "../game.h"
#include "entity.h"

Entity player_init(const Sprite* player_sprite, Game& g);
void start_animation(Entity& e, u32 anim_idx, bool should_loop = false, u64 frame_time = 100);
Update_Result player_update(Entity& p, Game& g);
void player_draw(SDL_Renderer* r, const Entity& p, Game& g);

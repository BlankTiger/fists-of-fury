#include "debug_menu.h"
#include "draw.h"
#include "settings.h"

#include <SDL3/SDL.h>

void debug_menu_update(Debug_Menu& dm) {}

void debug_menu_draw(const Debug_Menu& dm, SDL_Renderer* r) {
    const SDL_FRect dst = {0, 0, 15, 15};
    const Draw_Box_Opts opts = {
        .colors_border = settings.collision_box_colors_border,
        .colors_fill = settings.collision_box_colors_fill
    };
    draw_box(r, dst, opts);
}

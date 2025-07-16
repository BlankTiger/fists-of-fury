#include "debug_menu.h"
#include "draw.h"
#include "settings.h"

#include <SDL3/SDL.h>

void debug_menu_update(Debug_Menu& dm) {}

void debug_menu_draw(const Debug_Menu& dm, SDL_Renderer* r) {
    const SDL_FRect dst_box = {0, 0, dm.width_box, dm.height_box};
    const Draw_Box_Opts opts_box = {
        .colors_border = settings.colors_collision_box_border,
        .colors_fill = settings.colors_collision_box_fill
    };
    draw_box(r, dst_box, opts_box);
}

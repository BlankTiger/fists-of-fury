#include "debug_menu.h"
#include "draw.h"
#include "settings.h"

#include <SDL3/SDL.h>

#include <string>

void debug_menu_update(Debug_Menu& dm) {
    if (!dm.show) return;
}

static std::string debug_menu_get_content(const Debug_Menu& dm) {
    return "heyyyyy";
}

void debug_menu_draw(const Debug_Menu& dm, SDL_Renderer* r) {
    if (!dm.show) return;
    const SDL_FRect dst_box = {0, 0, dm.width_box, dm.height_box};
    const Draw_Box_Opts opts_box = {
        .colors_border = settings.colors_collision_box_border,
        .colors_fill = settings.colors_collision_box_fill
    };
    draw_box(r, dst_box, opts_box);

    const auto content = debug_menu_get_content(dm);
    const auto font_width_from_border = dm.width_border + dm.width_font;
    const auto width_max = dm.width_box - 2*dm.width_border; // will probably need some additional space for breathing
    const auto height_max = dm.height_box - 2*dm.width_border; // same thing here
    const SDL_FRect dst_text = {font_width_from_border, font_width_from_border, width_max, height_max};
    const Draw_Text_Opts opts_text = {
        .color = settings.color_text
    };
    draw_text(r, dst_text, opts_text);
}

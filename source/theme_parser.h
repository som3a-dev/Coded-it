#include "json_parser.h"
#include <SDL.h>
#include <stdbool.h>

json_value* tp_get_color_in_token_colors(json_array* token_colors, const char* token_str);

bool tp_load_color(json_object* parent_obj, const char* path, SDL_Color* color);

void tp_load_theme(SDL_Color* token_colors, SDL_Color* cursor_color, SDL_Color* bg_color, const char* theme_path);

#include "json_parser.h"
#include <SDL.h>

json_value* tp_get_color_in_token_colors(json_array* token_colors, const char* token_str);

void tp_load_theme(SDL_Color* token_colors, SDL_Color* bg_color, const char* theme_path);

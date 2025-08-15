#include "json_parser.h"
#include "editor.h"
#include <SDL.h>
#include <stdbool.h>

json_value* tp_get_color_in_token_colors(json_array* token_colors, const char* token_str);

bool tp_load_color(json_object* parent_obj, const char* path, SDL_Color* color);

bool tp_load_theme(ProgramState* state, const char* theme_path);


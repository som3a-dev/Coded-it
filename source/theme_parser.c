#include "theme_parser.h"
#include "syntax_parser.h"
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>


bool tp_load_color(json_object* parent_obj, const char* path, SDL_Color* color)
{
    if (color == NULL) return false;

    json_value* theme_color = jp_get_child_value_in_object(parent_obj, path);

    if (theme_color)
    {
        if (theme_color->type != JSON_VALUE_STRING)
        {
            return false;
        }

        char* str = theme_color->val;
        str++;
        str++;
        str[strlen(str)-1] = '\0';

        rgb_hex_str_to_int(str,
        &(color->r),
        &(color->g),
        &(color->b),
        &(color->a));

        return true;
    }
    
    return false;
}


void tp_load_theme(SDL_Color* token_colors, SDL_Color* cursor_color, SDL_Color* bg_color, const char* theme_path)
{
    if (token_colors == NULL) return;
    if (theme_path == NULL) return;

    //load theme file
    json_object* parent_obj = jp_parse_file(theme_path);
    if (parent_obj == NULL)
    {
        printf("Loading theme failed.\n");
        return;
    }

    //load background color
    tp_load_color(parent_obj, "colors/editor.background", bg_color);

    //Load cursor color
    if (cursor_color)
    {
        if (tp_load_color(parent_obj, "colors/editorCursor.foreground", cursor_color))
        {

        }
        else if (bg_color)
        {
            //Determine if the theme is light or dark with the background color
            float brightness = (0.2126 * bg_color->r + 0.7152 * bg_color->g + 0.0722 * bg_color->b) / 255;
            brightness = 1.0 - brightness;

            //230 not 255 because i didn't like the look of a fully white cursor. fully black is fine though
            cursor_color->r = 230 * brightness;
            cursor_color->g = 230 * brightness;
            cursor_color->b = 230 * brightness;
            cursor_color->a = 255;
        }
        else
        {
            //Default to a greyish cursor
            cursor_color->r = 150;
            cursor_color->g = 150;
            cursor_color->b = 150;
            cursor_color->a = 255;
        }
    }

    json_array* theme_token_colors = NULL;
    {
        json_value* token_colors_val = jp_get_child_value_in_object(parent_obj, "tokenColors");
        if (token_colors_val != NULL)
        {
            theme_token_colors = (json_array*)(token_colors_val->val);
        }
    }

    {
        SDL_Color* color = token_colors + TOKEN_NONE;
        
        if (tp_load_color(parent_obj, "colors/editor.foreground", color))
        {
        }
        else
        {
            color->r = 200;
            color->g = 200;
            color->b = 200;
            color->a = 255;
        }

    }
    {
        SDL_Color* color = token_colors + TOKEN_KEYWORD;
        json_value* token_color = tp_get_color_in_token_colors(theme_token_colors, "\"keyword\"");

        if (token_color)
        {
            assert(token_color->type == JSON_VALUE_STRING);
            char* str = token_color->val;
            str++;
            str++;
            str[strlen(str)-1] = '\0';

            rgb_hex_str_to_int(str, &(color->r), &(color->g), &(color->b), &(color->a));
        }
        else
        {
            color->r = 0x96;
            color->g = 0x4b;
            color->b = 0x00;
            color->a = 255;
        }
    }
    {
        SDL_Color* color = token_colors + TOKEN_NUMERIC;

        //TODO(omar): Maybe checking for this in semanticTokenColors isn't needed as it isn't in all themes.
        //But constant.numeric seems to be in all themes and the same value
        //But i'll keep it for now
        json_value* token_color = jp_get_child_value_in_object(parent_obj, "semanticTokenColors/numberLiteral");
    
        if (token_color == NULL)
        {
            token_color = tp_get_color_in_token_colors(theme_token_colors, "\"constant.numeric\"");
        }

        if (token_color)
        {
            assert(token_color->type == JSON_VALUE_STRING);
            char* str = token_color->val;
            str++;
            str++;
            str[strlen(str)-1] = '\0';

            rgb_hex_str_to_int(str, &(color->r), &(color->g), &(color->b), &(color->a));
        }
        else
        {
            color->r = 165;
            color->g = 255;
            color->b = 120;
            color->a = 255;
        }
    }
    {
        SDL_Color* color = token_colors + TOKEN_STRING_LITERAL;
        json_value* token_color = jp_get_child_value_in_object(parent_obj, "semanticTokenColors/stringLiteral");

        if (token_color == NULL)
        {
            token_color = tp_get_color_in_token_colors(theme_token_colors, "\"string\"");
        }

        if (token_color)
        {
            assert(token_color->type == JSON_VALUE_STRING);
            char* str = token_color->val;
            str++;
            str++;
            str[strlen(str)-1] = '\0';

            rgb_hex_str_to_int(str, &(color->r), &(color->g), &(color->b), &(color->a));
        }
        else
        {
            color->r = 50;
            color->g = 255;
            color->b = 60;
            color->a = 255;
        }
    }
    {
        SDL_Color* color = token_colors + TOKEN_BRACES;
        const char* dark_default = "FFD700";
        rgb_hex_str_to_int(dark_default, &(color->r), &(color->g), &(color->b), &(color->a));
    }
    {
        SDL_Color* color = token_colors + TOKEN_COMMENT;
        json_value* token_color = tp_get_color_in_token_colors(theme_token_colors, "\"comment\"");
        
        if (token_color)
        {
            assert(token_color->type == JSON_VALUE_STRING);
            char* str = token_color->val;
            str++;
            str++;
            str[strlen(str)-1] = '\0';

            rgb_hex_str_to_int(str, &(color->r), &(color->g), &(color->b), &(color->a));
        }
        else
        {
            color->r = 180;
            color->g = 180;
            color->b = 180;
            color->a = 255;
        }
    }
}


json_value* tp_get_color_in_token_colors(json_array* token_colors, const char* token_str)
{
    json_value* token_color = NULL;
    
    for (int i = 0; i < token_colors->values_count; i++) 
    {
        json_value* val = token_colors->values + i;
        if (val->type != JSON_VALUE_OBJECT)
        {
            //TODO(omar): this probably should never happen. maybe print an error
            assert(false);
            continue;
        }

        json_object* obj = (json_object*)(val->val);
        assert(obj);

        json_value* scope = hash_table_get(&(obj->table), "scope");
        if (scope == NULL)
        {
            //TODO(omar): this probably should never happen. maybe print an error
            assert(false);
            continue;
        }

        if (scope->type == JSON_VALUE_STRING)
        {
            if (strcmp(scope->val, token_str)== 0)
            {
                json_value* settings = hash_table_get(&(obj->table), "settings");
                assert(settings);
                json_object* settings_obj = (json_object*)(settings->val);

                token_color = hash_table_get(&(settings_obj->table), "foreground");
                return token_color;
            }
        }
        else if (scope->type == JSON_VALUE_ARRAY)
        {
            json_array* arr = (json_array*)(scope->val);
            for (int i = 0; i < arr->values_count; i++)
            {
                json_value* val = arr->values + i;
                if (val->type == JSON_VALUE_STRING)
                {
                    if (strcmp(val->val, token_str) == 0)
                    {
                        json_value* settings = hash_table_get(&(obj->table), "settings");
                        assert(settings);
                        json_object* settings_obj = (json_object*)(settings->val);

                        token_color = hash_table_get(&(settings_obj->table), "foreground");
                        return token_color;
                    }
                }
            }
        }
    }

    return token_color;
}
#include "editor_input_buffer.h"
#include "syntax_parser.h"


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))



InputBuffer* editor_get_current_input_buffer(const ProgramState* state)
{
    switch (state->state)
    {
        case EDITOR_STATE_EDIT:
        {
            return &(state->text);
        } break;

        case EDITOR_STATE_COMMAND_INPUT:
        {
            return &(state->command_input);
        } break;
    }

    return NULL;
}


void editor_draw_cursor(ProgramState* state, const TTF_Font* font)
{
    if (state->draw_cursor)
    {
        int char_w;
        int char_h;
        TTF_SizeText(font, "A", &char_w, &char_h);

        int cursor_x = 0;
        int cursor_y = 0;
        editor_get_cursor_pos(state, &cursor_x, &cursor_y, char_w, char_h);

        bool draw_cursor = true;

        if (state->state == EDITOR_STATE_EDIT)
        {
            cursor_x -= state->camera_x;
            cursor_y -= state->camera_y;
            if ((cursor_x + char_w) > state->editor_area.w)
            {
                draw_cursor = false;
            }
            if ((cursor_y + char_h) > state->editor_area.h)
            {
                draw_cursor = false;
            }
            if (cursor_y < state->editor_area.y)
            {
                draw_cursor = false;
            }
            if (cursor_x < state->editor_area.x)
            {
                draw_cursor = false;
            }
        }

        if (draw_cursor)
        {
            SDL_Rect cursor_rect = { cursor_x, cursor_y, char_w, char_h};
            if (state->selection_start_index != -2)
            {
                cursor_rect.y += char_h - 1;
                cursor_rect.h = char_h * 0.1;
            }
            SDL_FillRect(state->window_surface, &cursor_rect,
                        SDL_MapRGB(state->window_surface->format,
                        state->cursor_color.r,
                        state->cursor_color.g,
                        state->cursor_color.b));
        }
    }
}

bool editor_draw_input_buffer_character(ProgramState* state, 
                                        char c, int x, int y,
                                        int char_w, int char_h,
                                        int index_in_text,
                                        SDL_Color* color)
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return;

    TTF_Font* font = buffer->font;

    int selection_start = -2;
    int selection_end = -2;

    if (state->selection_start_index != -2)
    {
        if (state->selection_start_index < (buffer->cursor_index))
        {
            selection_start = MIN(state->selection_start_index, buffer->cursor_index);
            selection_end = MAX(state->selection_start_index, buffer->cursor_index);
        }
        else
        {
            selection_start = MIN(state->selection_start_index, buffer->cursor_index);
            selection_end = MAX(state->selection_start_index, buffer->cursor_index);
        }
    }

    char text[2] = {c, '\0'};
    int draw_x = x;
    int draw_y = y;

    if (state->state == EDITOR_STATE_EDIT)
    {
        draw_x -= state->camera_x;
        draw_y -= state->camera_y;

        if ((draw_y + char_h) > (state->editor_area.h + state->editor_area.y))
        {
            return false;
        }
        if ((draw_y) < state->editor_area.y)
        {
            return false;
        }
        if ((draw_x + char_w) > (state->editor_area.w + state->editor_area.x))
        {
            return false;
        }
        if ((draw_x) < state->editor_area.x)
        {
            return false;
        }
    }

    //check if part of selection start or end
    if ((selection_start <= index_in_text) &&
        (selection_end >= index_in_text))
    {
        SDL_Rect rect = { draw_x, draw_y, char_w, char_h};
        SDL_FillRect(state->window_surface, &rect,
        SDL_MapRGB(state->window_surface->format,
        200, 200, 200));

        draw_text(font, state->window_surface,
        text, draw_x, draw_y,
        color->r, color->g, color->b,
        200, 200, 200); //TODO(omar): pick a selection color from the theme
    }
    else if ((state->draw_cursor) && (index_in_text == buffer->cursor_index))
    {
        draw_text(font, state->window_surface,
        text, draw_x, draw_y,
        color->r, color->g, color->b,
        state->cursor_color.r, state->cursor_color.g, state->cursor_color.b);
    }
    else
    {
        draw_text(font, state->window_surface,
        text, draw_x, draw_y,
        color->r, color->g, color->b,
        state->bg_color.r, state->bg_color.g, state->bg_color.b);
    }

    return true;
}

void editor_draw_input_buffer(ProgramState* state) 
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return;

    TTF_Font* font = buffer->font;

    if (state->draw_cursor)
    {
        editor_draw_cursor(state, font);
    }

    int x = buffer->x;
    int y = buffer->y;

    int char_w;
    int char_h;
    TTF_SizeText(font, "A", &char_w, &char_h);
    
    bool line_is_comment = false;

    sp_metadata meta_data = {0};

    if (buffer->text.text)
    {
        if (state->state == EDITOR_STATE_COMMAND_INPUT)
        {
            SDL_Color* token_color = state->token_colors + TOKEN_NONE;

            draw_text(font, state->window_surface, buffer->text.text, x, y,
            token_color->r, token_color->g, token_color->b,
            state->bg_color.r, state->bg_color.g, state->bg_color.b);
        }
        else
        {
            String current_token = {0};
            for (int i = 0; i <= buffer->text.len; i++)
            {
                { //CULLING
                    int draw_x = x;
                    int draw_y = y;
                    if (state->state == EDITOR_STATE_EDIT)
                    {
                        draw_x -= state->camera_x;
                        draw_y -= state->camera_y;
                        if ((draw_y) > (state->editor_area.h + state->editor_area.y))
                        {
                            goto end_text_rendering;
                        }
                    }
                }

                {
                    switch (buffer->text.text[i])
                    {
                        case ' ':
                        case '\n':
                        case '(':
                        case ')':
                        case '{':
                        case '}':
                        case '[':
                        case ']':
                        case ';':
                        case '\0':
                        {
                            if (current_token.text != NULL)
                            {
                                int token_type = sp_get_token_type(current_token.text, &meta_data);

                                if (token_type == TOKEN_COMMENT)
                                {
                                    line_is_comment = true;
                                }

                                if (line_is_comment)
                                {
                                    token_type = TOKEN_COMMENT;
                                }

                                SDL_Color* token_color = state->token_colors + token_type;

                                bool lock_token_color = true;

                                if ((token_type != TOKEN_COMMENT) && (token_type != TOKEN_STRING_LITERAL))
                                {
                                    lock_token_color = false;
                                }

                                //Draw token text
                                for (int j = 0; j < current_token.len; j++)
                                {
                                    int char_index_in_text = j + (i - current_token.len);
                                    char c = current_token.text[j];
                                    SDL_Color* color = token_color;

                                    if (c == '"')
                                    {
                                        meta_data.quote_count++;
                                    }

                                    if (lock_token_color)
                                    {
                                        goto draw_token_char;
                                    }
                                    else
                                    {
                                        //Check for a string literal inside the token
                                        if (((meta_data.quote_count % 2) != 0) || (c == '"'))
                                        {
                                            color = state->token_colors + TOKEN_STRING_LITERAL;
                                        }

                                        //Check for a comment inside the token
                                        else if (c == '/')
                                        {
                                            if (j < (current_token.len-1))
                                            {
                                                if ((current_token.text[j+1] == c))
                                                {
                                                    lock_token_color = true;
                                                    token_color = state->token_colors + TOKEN_COMMENT;
                                                    color = token_color;
                                                }
                                            }
                                        }
                                    }

                                    draw_token_char:
                                    editor_draw_input_buffer_character(state, current_token.text[j],
                                    x, y, char_w, char_h, char_index_in_text, color);

                                    x += char_w;
                                }

                                String_clear(&current_token);

                            }

                            //draw the delimiter
                            if (buffer->text.text[i] == '\0')
                            {
                                continue;
                            }

                            switch (buffer->text.text[i])
                            {
                                case ' ':
                                {
                                    x += char_w;
                                } break;

                                case '\n':
                                {
                                    y += char_h; 
                                    x = buffer->x;

                                    line_is_comment = false;
                                } break;

                                default:
                                {
                                    char text[2] = {buffer->text.text[i], '\0'};
                                    int token_type;

                                    if (line_is_comment)
                                    {
                                        token_type = TOKEN_COMMENT;
                                    }
                                    else
                                    {
                                        token_type = sp_get_token_type(text, &meta_data);
                                    }

                                    SDL_Color* token_color = state->token_colors + token_type;

                                    editor_draw_input_buffer_character(state, buffer->text.text[i],
                                    x, y, char_w, char_h, i, token_color);

                                    x += char_w;
                                } break;
                            }
                        } break;

                        default:
                        {
                            if (buffer->text.text[i] != '\0')
                            {
                                String_push(&current_token, buffer->text.text[i]);
                            }
                        } break;
                    }
                }
            }
            end_text_rendering:;

            String_clear(&current_token);
        }
    }

}

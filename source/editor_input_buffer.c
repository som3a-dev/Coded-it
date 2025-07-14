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
        int char_w = state->char_w;
        int char_h = state->char_h;
        TTF_SizeText(font, "A", &char_w, &char_h);

        int cursor_x = 0;
        int cursor_y = 0;
        editor_get_cursor_pos(state, &cursor_x, &cursor_y, char_h);

        bool draw_cursor = true;

        if (state->state == EDITOR_STATE_EDIT)
        {
            cursor_x -= state->camera_x;
            cursor_y -= state->camera_y;
            if ((cursor_x + char_w) > state->editor_area_w)
            {
                draw_cursor = false;
            }
            if ((cursor_y + char_h) > state->editor_area_h)
            {
                draw_cursor = false;
            }
            if (cursor_y < state->editor_area_y)
            {
                draw_cursor = false;
            }
            if (cursor_x < state->editor_area_x)
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


void editor_draw_input_buffer(ProgramState* state) 
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return;

    TTF_Font* font;

    if (state->state == EDITOR_STATE_COMMAND_INPUT)
    {
        font = state->static_font;
    }
    else
    {
        font = state->font;
    }

    if (state->draw_cursor)
    {
        editor_draw_cursor(state, font);
    }

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

    int x = buffer->x;
    int y = buffer->y;
   

    if (buffer->text.text)
    {
        String current_token = {0};
        for (int i = 0; i <= buffer->text.len; i++)
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
                        int token_type = sp_get_token_type(current_token.text);
                        SDL_Color* token_color = state->token_colors + token_type;

                        //Draw token text
                        for (int j = 0; j < current_token.len; j++)
                        {
                            char text[2] = {current_token.text[j], '\0'};

                            int draw_x = x;
                            int draw_y = y;
                            if (state->state == EDITOR_STATE_EDIT)
                            {
                                draw_x -= state->camera_x;
                                draw_y -= state->camera_y;
                            }

                            int char_w;
                            int char_h;
                            TTF_SizeText(font, text, &char_w, &char_h);

                            if (state->state == EDITOR_STATE_EDIT)
                            {
                                if ((draw_y + char_h) > state->editor_area_h)
                                {
                                    goto end_text_rendering;
                                }
                            }

                            //check if part of selection start or end
                            int char_index_in_text = j + (i - current_token.len);
                            if ((selection_start <= char_index_in_text) &&
                                (selection_end >= char_index_in_text))
                            {
                                SDL_Rect rect = { draw_x, draw_y, char_w, char_h};
                                SDL_FillRect(state->window_surface, &rect,
                                SDL_MapRGB(state->window_surface->format,
                                200, 200, 200));

                                draw_text(font, state->window_surface,
                                text, draw_x, draw_y,
                                token_color->r, token_color->g, token_color->b,
                                200, 200, 200); 
                            }
                            else if (char_index_in_text == buffer->cursor_index)
                            {
                                draw_text(font, state->window_surface,
                                text, draw_x, draw_y,
                                token_color->r, token_color->g, token_color->b,
                                state->cursor_color.r, state->cursor_color.g, state->cursor_color.b);
                            }
                            else
                            {
                                draw_text(font, state->window_surface,
                                text, draw_x, draw_y,
                                token_color->r, token_color->g, token_color->b,
                                state->bg_color.r, state->bg_color.g, state->bg_color.b);
                            }

                            x += char_w;
                        }

                        String_clear(&current_token);
                    }

                    //draw the delimiter
                    if (buffer->text.text[i] == '\0')
                    {
                        continue;
                    }
                    int draw_x = x;
                    int draw_y = y;
                    if (state->state == EDITOR_STATE_EDIT)
                    {
                        draw_x -= state->camera_x;
                        draw_y -= state->camera_y;
                    }

                    int char_w;
                    int char_h;
                    {
                        char text[2] = {buffer->text.text[i], '\0'};
                        TTF_SizeText(font, text, &char_w, &char_h);
                    }

                    //draw selection highlight
                    if ((selection_start <= i) &&
                        (selection_end >= i))
                    {
                        SDL_Rect rect = { draw_x, draw_y, char_w, char_h};
                        SDL_FillRect(state->window_surface, &rect,
                        SDL_MapRGB(state->window_surface->format,
                        200, 200, 200));
                    }

                    switch (buffer->text.text[i])
                    {
                        case ' ':
                        {
                            x += char_w;
                        } break;

                        case '\n':
                        {
                            y += state->char_h; 
                            x = buffer->x;
                        } break;

                        default:
                        {
//                            printf("%c", buffer->text.text[i]);

                            bool draw_delimiter = true;
                            if (state->state == EDITOR_STATE_EDIT)
                            {
                                if ((draw_x + char_w) > state->editor_area_w)
                                {
                                    draw_delimiter = false;
                                }
                                if ((draw_y + char_h) > state->editor_area_h)
                                {
                                    draw_delimiter = false;
                                }
                                if (draw_y < state->editor_area_y)
                                {
                                    draw_delimiter = false;
                                }
                                if (draw_x < state->editor_area_x)
                                {
                                    draw_delimiter = false;
                                }
                            }

                            if (draw_delimiter)
                            {
                                char text[2] = {buffer->text.text[i], '\0'};

                                int token_type = sp_get_token_type(text);
                                SDL_Color* token_color = state->token_colors + token_type;

                                if ((selection_start <= i) &&
                                    (selection_end >= i))
                                {
                                    SDL_Rect rect = { draw_x, draw_y, char_w, char_h};
                                    SDL_FillRect(state->window_surface, &rect,
                                    SDL_MapRGB(state->window_surface->format,
                                    200, 200, 200));

                                    draw_text(font, state->window_surface,
                                    text, draw_x, draw_y,
                                    token_color->r, token_color->g, token_color->b,
                                    200, 200, 200); 
                                }
                                else if (i == buffer->cursor_index)
                                {
                                    draw_text(font, state->window_surface,
                                    text, draw_x, draw_y,
                                    token_color->r, token_color->g, token_color->b,
                                    state->cursor_color.r, state->cursor_color.g, state->cursor_color.b);
                                }
                                else
                                {
                                    draw_text(font, state->window_surface,
                                    text, draw_x, draw_y,
                                    token_color->r, token_color->g, token_color->b,
                                    state->bg_color.r, state->bg_color.g, state->bg_color.b);
                                }
                            }
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
            end_text_rendering:;
        }
//        printf("\n\n");

        String_clear(&current_token);
    }


}

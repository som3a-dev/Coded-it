#include "editor_input_buffer.h"


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


void editor_draw_input_buffer(ProgramState* state) 
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return;

    TTF_Font* font;
    int char_w;
    int char_h;

    if (state->state == EDITOR_STATE_COMMAND_INPUT)
    {
        font = state->static_font;
        TTF_SizeText(font, "A", &char_w, &char_h);
    }
    else
    {
        font = state->font;
        char_w = state->char_w;
        char_h = state->char_h;
    }

    int startx = buffer->x;
    int starty = buffer->y;

    if (state->draw_cursor)
    {
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
                        SDL_MapRGB(state->window_surface->format, 200, 200, 200));
        }
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

    int x = startx;
    int y = starty;
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
                case '\0':
                {
                    if (current_token.text != NULL)
                    {
                        int draw_x = x;
                        int draw_y = y;
                        if (state->state == EDITOR_STATE_EDIT)
                        {
                            draw_x -= state->camera_x;
                            draw_y -= state->camera_y;
                        }


                        printf("%s", current_token.text);
                        //Draw token text
                        draw_text(font, state->window_surface,
                        current_token.text, draw_x, draw_y, 255, 255, 255);
                        
                        int token_text_w;
                        TTF_SizeText(font, current_token.text, &token_text_w, NULL);
                        x += token_text_w;

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
                    {
                        char text[2] = {buffer->text.text[i], '\0'};
                        TTF_SizeText(font, text, &char_w, NULL);
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
                            x = startx;
                        } break;

                        default:
                        {
                            printf("%c", buffer->text.text[i]);

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
                                draw_text(font, state->window_surface,
                                text, draw_x, draw_y, 255, 255, 255);
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
        }
        printf("\n\n");

        String_clear(&current_token);
    }
}

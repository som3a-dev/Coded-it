#include "editor.h"
#include "editor_event.h"
#include "editor_fileio.h"
#include "editor_input_buffer.h"

#include "util.h"
#include "button.h"
#include "button_callback.h"
#include "draw.h"
#include "json_parser.h"
#include "syntax_parser.h"
#include "theme_parser.h"

#include <assert.h>
#include <direct.h>
#include <memory.h>
#include <windows.h>
#include <fileapi.h>
#include <stdio.h>
#include <string.h>
#include <SDL_syswm.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

const int CURSOR_BLINK_TIME = 1000;
const int MESSAGE_DURATION = 1000;

void editor_init(ProgramState* state)
{
    memset(state, 0, sizeof(ProgramState));
    state->running = true;
    state->draw_cursor = true;
    state->state = EDITOR_STATE_EDIT;

    const char* error = NULL;
     
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    error = SDL_GetError();
    if (error[0])
    {
        printf("Initializing SDL failed!\nError Message: '%s'\n", error);
        return 1;
    }

    state->window_w = 800;
    state->window_h = 600; 
    state->window = SDL_CreateWindow("Coded-it", 100, 100, state->window_w, state->window_h,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!state->window)
    {
        printf("SDL window creation failed!\n");
        return 2;
    }
    state->window_surface = SDL_GetWindowSurface(state->window);
    
    TTF_Init();
    error = TTF_GetError();
    if (error[0])
    {
        printf("Initializing SDL_ttf failed!\nError Message: '%s'\n", error);
        return 3;
    }

    state->font_size = 18;
    state->font = TTF_OpenFont("CONSOLA.ttf", state->font_size);
    state->ui_font_size = 16;
    state->ui_font = TTF_OpenFont("CONSOLA.ttf", state->ui_font_size);
    state->file_explorer_font_size = 16;
    state->file_explorer_font = TTF_OpenFont("CONSOLA.ttf", state->file_explorer_font_size);
    if (!(state->font) || !(state->ui_font) || !(state->file_explorer_font))
    {
        printf("Loading Font Failed\nError Message: %s\n", TTF_GetError());
        return 4;
    }

    TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));

    ButtonConfig config = {0};
    config.pressed_r = 110;
    config.pressed_g = 100;
    config.pressed_b = 100;
    config.font = state->ui_font;
    config.h = state->char_h;
    config.w = state->window_w;
    config.x = 0;
    config.y = 0;

    int index = 0;
    {
/*        config.text = "File";
        config.on_click = Button_file_on_click;
        config.on_input = NULL;
        config.disabled = false;
        Button_init(state->buttons + index, &config);
        index++;*/
    }
    {
        config.text = "Open File";

        config.on_click = Button_open_on_click;
        config.on_input = Button_open_on_input;
        config.disabled = false;
        Button_init(state->buttons + index, &config);

//        Button_add_child(get_button_by_text(state->buttons, 10, "File"), state, index);
 
        index++;
        config.y += config.h;

    }
    {
        config.text = "Save File";

        config.on_click = Button_save_on_click;
        config.on_input = Button_save_on_input;
        config.disabled = false;
        Button_init(state->buttons + index, &config);

//        Button_add_child(get_button_by_text(state->buttons, 10, "File"), state, index);

        index++;
        config.y += config.h;
    }
    {
        config.text = "Load Theme";

        config.on_click = Button_load_theme_on_click;
        config.disabled = false;
        Button_init(state->buttons + index, &config);

//        Button_add_child(get_button_by_text(state->buttons, 10, "File"), state, index);

        index++;
        config.y += config.h;
    }


    Stack_init(&(state->undo_tree), sizeof(TextAction));
    Stack_init(&(state->redo_tree), sizeof(TextAction));
    Queue_init(&(state->messages), sizeof(String));

    String msg = {0};
    String_set(&msg, "Initialized.");
    editor_push_message(state, &msg);

    state->selection_start_index = -2;

//    state->current_file = "json_parser.c";
//    editor_open_file(state);

    state->token_colors = malloc(sizeof(SDL_Color) * _TOKEN_COUNT);
    tp_load_theme(state, "theme.json");

    //draw areas
    state->editor_area.border_thickness = 2;

    state->message_area.flags |= DRAW_AREA_TOP_BORDER | DRAW_AREA_FILL;
    state->message_area.border_thickness = 2;

    state->file_explorer_area.border_thickness = 4;
    state->file_explorer_area.flags |= DRAW_AREA_RIGHT_BORDER | DRAW_AREA_BOTTOM_BORDER | DRAW_AREA_TOP_BORDER; 
    state->file_explorer_area.outline_color.r = 50;
    state->file_explorer_area.outline_color.g = 50;
    state->file_explorer_area.outline_color.b = 50;

    state->status_bar_area.flags |= DRAW_AREA_FILL;
    state->status_bar_area.border_thickness = 2;

    //Must be called to init draw areas and stuff
    editor_resize_and_reposition(state); //to set editor_area.h

    //Input buffer properties
    state->text.font = state->font;
    state->command_input.font = state->ui_font;
    state->current_directory_ib.font = state->ui_font;

    editor_set_state(state, EDITOR_STATE_EDIT);


    //Directory stuff
    //Get working directory
    state->current_directory_ib.text.text = _getcwd(NULL, 0);
    assert(state->current_directory_ib.text.text && "_getcwd failed.\n");
    state->current_directory_ib.text.len = strlen(state->current_directory_ib.text.text);
    state->current_directory_ib.cursor_index = state->current_directory_ib.text.len;

    editor_update_file_explorer(state);

    editor_open_file(state, "CODE.c");

    state->tab_size = 4;

}


void editor_destroy(ProgramState* state)
{
    TTF_CloseFont(state->font);
    TTF_CloseFont(state->ui_font);
    TTF_CloseFont(state->file_explorer_font);

    free(state->file_buttons);
    free(state->token_colors);
    String_clear(&(state->text.text));
    String_clear(&(state->command_input.text));
    String_clear(&(state->clipboard));
    String_clear(&(state->current_directory_ib.text));
    String_clear(&(state->current_directory));
    String_clear(&(state->current_file));


    TTF_Quit();
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}


void editor_loop(ProgramState* state)
{
    while (state->running)
    {
        bool should_update = false;
        editor_handle_events(state, &should_update);

        //check for timed interrupts/things we have to do. if its time to do something then
        //we should update
        editor_do_timed_events(state, &should_update);
        
        if (should_update)
        {
            editor_update(state);
            editor_draw(state);
//            printf("update\n");
        }
    }
}


void editor_do_timed_events(ProgramState* state, bool* should_update)
{
    if (state->selection_start_index == -2)
    {
        if ((SDL_GetTicks() - state->last_cursor_blink_tic) >= CURSOR_BLINK_TIME)
        {
            state->draw_cursor = !(state->draw_cursor);
            state->last_cursor_blink_tic = SDL_GetTicks();
            *should_update = true;
        }
    }
}


void editor_update(ProgramState* state)
{
    if (editor_get_current_input_buffer(state))
    {
        //Do stuff that is specific to states with an input buffer
        if (state->selection_start_index != -2)
        {
            state->draw_cursor = true;
        }
    }

    if (state->message)
    {
        int tic = SDL_GetTicks();
        if (tic > (state->message_change_tic + MESSAGE_DURATION))
        {
//            printf("Msg: %s\nChange tic: %d\nCurrent Tic: %d\nDuration: %d\n\n", state->message->text,
//            state->message_change_tic, tic, MESSAGE_DURATION);
            String_clear(state->message);
            free(state->message);
            state->message = NULL;
            state->message_change_tic = tic; 
        }
    }

    int mouse_x, mouse_y;
    uint32_t mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);

    switch (state->state)
    {
        case EDITOR_STATE_EDIT:
        {
            if (mouse_state & SDL_BUTTON(1))
            {
               mouse_x -= state->text.x;
               mouse_y -= state->text.y;
               if ((mouse_x >= 0) && (mouse_y >= 0)) 
               {
                    if (state->text.text.len != 0)
                    {
                        int mouse_char_x = mouse_x / state->char_w;
                        int mouse_char_y = mouse_y / state->char_h;

                        int char_x = 0;
                        int char_y = 0;
                        for (int i = 0; i <= state->text.text.len; i++)
                        {
                            if ((mouse_char_x == char_x) && (mouse_char_y == char_y))
                            {
                                editor_set_cursor(state, i);
                                break;
                            }

                            char_x++;

                            if (state->text.text.text[i] == '\n')
                            {
                                char_x = 0;
                                char_y++;
                            }
                        }
                    }
                }
            }

            int cursor_x = 0;
            int cursor_y = 0;
            editor_get_cursor_pos(state, &cursor_x, &cursor_y, state->char_w, state->char_h);
            cursor_x -= state->camera_x;
            cursor_y -= state->camera_y;

            if ((cursor_x + state->char_w) > state->editor_area.w)
            {
                state->camera_x += (((cursor_x + state->char_w) - state->editor_area.w) / state->char_w) * state->char_w;
            }
            if ((cursor_y + state->char_h) > state->editor_area.h)
            {
                state->camera_y += ((((cursor_y + state->char_h) - state->editor_area.h) / state->char_h) + 1) * state->char_h;
//                state->camera_y += state->char_h; //the cursor's height offset
            }

            if (cursor_y < state->editor_area.y)
            {
                state->camera_y += (((cursor_y) - state->editor_area.y) / state->char_h) * state->char_h;
            }
            if (cursor_x < state->editor_area.x)
            {
                state->camera_x += (((cursor_x) - state->editor_area.x) / state->char_w) * state->char_w;
            }

        } break;

        case EDITOR_STATE_FILE_EXPLORER:
        {
            if (mouse_state)
            {
                editor_check_button_mouse_click(state, state->file_buttons, state->file_count);
            }
        } break;

        case EDITOR_STATE_COMMAND:
        {
            if (mouse_state)
            {
                editor_check_button_mouse_click(state, state->buttons, 10);
            }
        } break;
    }
}


void editor_update_file_explorer(ProgramState* state)
{
    //Check for a valid current directory in the input buffer
    int attributes = GetFileAttributes(state->current_directory_ib.text.text);
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        if (state->current_directory.text && state->current_directory_ib.text.text)
        {
            if (strcmp(state->current_directory.text, state->current_directory_ib.text.text) == 0)
            {
                return;
            }
        }
        String_set(&(state->current_directory), state->current_directory_ib.text.text);
    }
    else
    {
        return;
    }

    if (state->current_directory.text == NULL) return;

    if (state->state == EDITOR_STATE_FILE_EXPLORER)
    {
        if (state->clicked_button)
        {
            state->clicked_button = NULL;
        }
    }
    free(state->file_buttons);
    state->file_buttons = NULL;
    state->file_count = 0;

    //Load directory files
    WIN32_FIND_DATAA data = {0};
    const char* suffix = "\\*";
    
    String path = {0};
    String_set(&path, state->current_directory.text);
    String_insert_string(&path, suffix, path.len);

    HANDLE dir_handle = FindFirstFileA(path.text, &data);
    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    while (FindNextFileA(dir_handle, &data) != 0)
    {
        editor_add_file_to_explorer(state, data.cFileName);
    }

    if (state->state == EDITOR_STATE_FILE_EXPLORER)
    {
        editor_select_first_enabled_button(state, state->file_buttons, state->file_count);
    }    

    String_clear(&path);

    state->file_explorer_camera_x = 0;
    state->file_explorer_camera_y = 0;
}


void editor_draw(ProgramState* state)
{
    SDL_FillRect(state->window_surface, NULL,
    SDL_MapRGB(state->window_surface->format,
    state->bg_color.r, state->bg_color.g, state->bg_color.b)); //Clear

    switch (state->state)
    {
        case EDITOR_STATE_COMMAND:
        {
            for (int i = 0; i < 10; i++)
            {
                Button_draw(state->buttons + i, state->window_surface, &(state->bg_color),
                0, 0);
            }
        } break;

        case EDITOR_STATE_FILE_EXPLORER:
        {
            editor_draw_file_explorer(state);
        } break;

        case EDITOR_STATE_EDIT:
        {
            editor_draw_input_buffer(state);
            editor_draw_status_bar(state);
        } break;
    }

    {
        editor_render_draw_area(state, &(state->message_area));

        if (!state->message)
        {
            state->message = Queue_pop(&(state->messages), true);
            if (state->message)
            {
                state->message_change_tic = SDL_GetTicks();
            }
        }
        if (state->message)
        {
            int char_h;
            TTF_SizeText(state->ui_font, "A", NULL, &char_h);

            draw_text(state->ui_font, state->window_surface, state->message->text,
                    0,
                    state->command_input.y,
                    233, 230, 230, 255,
                    state->bg_color.r, state->bg_color.g, state->bg_color.b);

        }
    }

/*    SDL_Rect rect = {0, 0, 200, 200};
    static int a = 0;
    a++;
    SDL_Color color = {200, 200, 200, a};
    SDL_FillRectAlpha(state->window_surface, &rect, &color);*/

    SDL_UpdateWindowSurface(state->window);
}


void editor_draw_status_bar(ProgramState* state)
{
//    SDL_FillRect(state->window_surface, &(state->status_bar_area), SDL_MapRGB(state->window_surface->format, 170, 170, 170));
    editor_render_draw_area(state, &(state->status_bar_area));

    { //Line and col
        int line;
        int col;
        editor_get_cursor_pos(state, &col, &line, state->char_w, state->char_h);
        line -= state->text.y;
        col -= state->text.x;
        line /= state->char_h;
        col /= state->char_w;

        const char* format = "Ln %d, Col %d";

        int text_len = (strlen(format) - 4) + ulen_helper(line) + ulen_helper(col) + 1;
        char* text = malloc(sizeof(char) * text_len);

        snprintf(text, text_len, format, line, col);

        int text_w;
        TTF_SizeText(state->font, text, &text_w, NULL);

//        int x = state->status_bar_area.x + (state->status_bar_area.w * 0.3f);
        int x = state->window_w - text_w;
        int y = state->status_bar_area.y + ((state->status_bar_area.h / 2) - (state->char_h / 2));


        draw_text(state->font, state->window_surface, text,
                    x, y, 
                    state->status_bar_area.text_color.r,
                    state->status_bar_area.text_color.g,
                    state->status_bar_area.text_color.b,
                    state->status_bar_area.text_color.a,
                    state->status_bar_area.color.r,
                    state->status_bar_area.color.g,
                    state->status_bar_area.color.b
        );
        free(text);
    }

    { //Current filename
        int x = state->status_bar_area.x + (state->status_bar_area.w * 0.3f);
        int y = state->status_bar_area.y + ((state->status_bar_area.h / 2) - (state->char_h / 2));
        x = state->status_bar_area.x;

        char* text = state->current_file.text;
        if (text == NULL)
        {
            text = "(No file)";
        }

        draw_text(state->font, state->window_surface, state->current_file.text,
                x, y, 
                state->status_bar_area.text_color.r,
                state->status_bar_area.text_color.g,
                state->status_bar_area.text_color.b,
                state->status_bar_area.text_color.a,
                state->status_bar_area.color.r,
                state->status_bar_area.color.g,
                state->status_bar_area.color.b);
    }
}


void editor_draw_file_explorer(ProgramState* state)
{
    for (int i = 0; i < state->file_count; i++)
    {
        int x = state->file_buttons[i].x - state->file_explorer_camera_x;
        int y = state->file_buttons[i].y - state->file_explorer_camera_y;
        int w = state->file_buttons[i].w;
        int h = state->file_buttons[i].h;

        if ((y) > (state->file_explorer_area.y + state->file_explorer_area.h))
        {
            continue;
        }
        if ((y+h) < state->file_explorer_area.y)
        {
            continue;
        }

        Button_draw(state->file_buttons + i,
        state->window_surface, &(state->bg_color), state->file_explorer_camera_x,
        state->file_explorer_camera_y);
    }


    editor_render_draw_area(state, &(state->file_explorer_area));

//    draw_text(state->ui_font, state->window_surface, state->current_directory_ib.text.text, 0, 0, 255, 255, 255,
//    state->bg_color.r, state->bg_color.g, state->bg_color.b);
    editor_draw_input_buffer(state);
}


void editor_render_draw_area(ProgramState* state, const DrawArea* area)
{
    if (area->flags & DRAW_AREA_FILL)
    {
//        SDL_FillRect(state->window_surface, area, color);
        SDL_FillRectAlpha(state->window_surface, area, &(area->color));
    }

    if ((area->flags & DRAW_AREA_TOP_BORDER) || (area->flags & DRAW_AREA_OUTLINE))
    {
        SDL_Rect rect = {
            area->x, area->y,
            area->w + area->border_thickness, area->border_thickness
        };

        SDL_FillRectAlpha(state->window_surface, &rect, &(area->outline_color));
    }

    if ((area->flags & DRAW_AREA_BOTTOM_BORDER) || (area->flags & DRAW_AREA_OUTLINE))
    {
        SDL_Rect rect = {
            area->x, area->y + area->h,
            area->w + area->border_thickness, area->border_thickness
        };

        SDL_FillRectAlpha(state->window_surface, &rect, &(area->outline_color));
    }

    if ((area->flags & DRAW_AREA_LEFT_BORDER) || (area->flags & DRAW_AREA_OUTLINE))
    {
        SDL_Rect rect = {
            area->x, area->y,
            area->border_thickness, area->h + area->border_thickness
        };

        SDL_FillRectAlpha(state->window_surface, &rect, &(area->outline_color));
    }

    if ((area->flags & DRAW_AREA_RIGHT_BORDER) || (area->flags & DRAW_AREA_OUTLINE))
    {
        SDL_Rect rect = {
            area->x + area->w, area->y,
            area->border_thickness, area->h + area->border_thickness
        };

        SDL_FillRectAlpha(state->window_surface, &rect, &(area->outline_color));
    }
}


void editor_add_file_to_explorer(ProgramState* state, const char* filename)
{
    state->file_count++;
    if (state->file_buttons == NULL)
    {
        state->file_buttons = calloc(state->file_count, sizeof(Button));
    }
    else
    {
        state->file_buttons = realloc(state->file_buttons,
        sizeof(Button) * state->file_count);
    }

    Button* button = state->file_buttons + ((state->file_count)-1);
    ButtonConfig cfg = {0};

    cfg.on_click = Button_file_name_on_click;

    cfg.text = filename;
    cfg.font = state->file_explorer_font;

    cfg.pressed_r = 110;
    cfg.pressed_g = 100;
    cfg.pressed_b = 100;

    Button_init(button, &cfg);

    editor_position_file_button(state, button, state->file_count);
}


void editor_position_file_button(const ProgramState* state, Button* button, int i)
{
    TTF_SizeText(state->file_explorer_font, button->text, NULL, &(button->h));

    int margin_between_file_names = button->h * MARGIN_BETWEEN_FILE_NAMES_FACTOR;

    button->w = state->window_w;

    button->x = state->file_explorer_area.x;
    button->y = state->file_explorer_area.y + state->file_explorer_area.border_thickness;
    if (i > 1)
    {
        button->y += (button->h + margin_between_file_names) * (i-1);
    }

    Button_resize_text(button, state->file_explorer_font);
}



void editor_check_button_mouse_click(ProgramState* state, Button* buttons, int button_count)
{
    state->clicked_button = NULL;
    for (int i = 0; i < button_count; i++)
    {
        Button* button = buttons + i;
        if (button->state == BUTTON_STATE_ENABLED)
        {
            if (Button_is_mouse_hovering(button))
            {
                //do
                if (button->on_click)
                {
                    button->on_click(button, state);
                    state->clicked_button = button;
                    break;
                }
            }
        }
    }
}


void draw_text(TTF_Font* font, SDL_Surface* dst_surface, const char* text,
                int x, int y, int r, int g, int b, int a,
                int br, int bg, int bb)
{
    if (text == NULL)
    {
        return;
    }

    SDL_Color text_color = { r, g, b, a };
    SDL_Color bg_color = {br, bg, bb, 255 };
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, text_color);

    if (text_surface == NULL)
    {
        return;
    }
    
    SDL_Rect text_dst = { x, y, 0, 0 };
    TTF_SizeText(font, text, &(text_dst.w), &(text_dst.h));
    
    SDL_BlitSurface(text_surface, NULL, dst_surface, &text_dst);

    SDL_FreeSurface(text_surface);
}


void editor_set_cursor(ProgramState* state, int index)
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    buffer->cursor_index = index;
    if (buffer->cursor_index < 0)
    {
        buffer->cursor_index = 0;
    }
    if (buffer->cursor_index > buffer->text.len)
    {
        buffer->cursor_index = buffer->text.len;
    }

    editor_get_cursor_pos(state, &(buffer->cursor_col), &(buffer->cursor_line), state->char_w,
                          state->char_h);
    
    //don't blink while typing
    state->last_cursor_blink_tic = SDL_GetTicks();
    state->draw_cursor = true;
}


void editor_set_state(ProgramState* state, int new_state)
{
    state->clicked_button = NULL;

    switch (state->state)
    {
        case EDITOR_STATE_EDIT:
        {
        } break;

        case EDITOR_STATE_FILE_EXPLORER:
        {
            state->file_explorer_camera_x = 0;
            state->file_explorer_camera_y = 0;

            for (int i = 0; i < state->file_count; i++)
            {
                Button* button = state->file_buttons + i;
                button->state = BUTTON_STATE_DISABLED;
            }
        } break;

/*        case EDITOR_STATE_COMMAND_INPUT:
        {
            InputBuffer* buffer = editor_get_current_input_buffer(state);
            if (state->clicked_button)
            {
                if (state->clicked_button->on_input)
                {
                    state->clicked_button->on_input(state->clicked_button, state, &(buffer->text));
                }
                state->clicked_button->mouse_hovering = false;
                state->clicked_button = NULL;
            }
            String_clear(&(buffer->text));
            buffer->cursor_index = 0;
        } break;*/

        case EDITOR_STATE_COMMAND:
        {
            //reset to main buttons like File
            for (int i = 0; i < 10; i++)
            {
                Button* button = state->buttons + i;
                button->mouse_hovering = false;
            } 

            {
                if (state->clicked_button)
                {
                    state->clicked_button->mouse_hovering = false;
                    state->clicked_button = NULL;
                }
            }
        } break;
    }

    if (new_state == EDITOR_STATE_FILE_EXPLORER)
    {
        state->file_explorer_area.w = state->window_w;

        for (int i = 0; i < state->file_count; i++)
        {
            state->file_buttons[i].mouse_hovering = false;
            state->file_buttons[i].state = BUTTON_STATE_ENABLED;
        }

        editor_select_first_enabled_button(state, state->file_buttons, state->file_count);
    }
    else if (new_state == EDITOR_STATE_COMMAND)
    {
        for (int i = 0; i < 10; i++)
        {
            state->buttons[i].mouse_hovering = false;
        }

        editor_select_first_enabled_button(state, state->buttons, 10);
    }
    else
    {
        //State has no buttons. disable this shit
        for (int i = 0; i < state->file_count; i++)
        {
            Button* button = state->file_buttons + i;
            button->mouse_hovering = false;
        }

        for (int i = 0; i < 10; i++)
        {
            Button* button = state->buttons + i;
            button->mouse_hovering = false;
        } 
    }

    state->state = new_state;
}


void editor_set_filename(ProgramState* state, const char* new_filename)
{
    String_set(&(state->current_file), new_filename);
}


void editor_resize_and_reposition(ProgramState* state)
{
    int ui_font_char_h;
    TTF_SizeText(state->ui_font, "A", NULL, &ui_font_char_h);
    int explorer_font_char_h;
    TTF_SizeText(state->file_explorer_font, "A", NULL, &explorer_font_char_h);

    //Ui elements and DrawAreas
    {
        //status bar area

        //TODO(omar): find a different way to indicate the first time initializing
        if (state->status_bar_area.x == 0)
        {
            state->status_bar_area.x = state->char_w * 0.1f;
            state->status_bar_area.y = state->char_h * 0.1f;
        }
        state->status_bar_area.w = state->window_w;
        state->status_bar_area.h = state->char_h * 1;

        //message area
        state->message_area.h = ui_font_char_h * 1.5f;
        state->message_area.y = state->window_h - state->message_area.h;
        state->message_area.w = state->window_w;

        //editor area
        state->editor_area.y = state->status_bar_area.y + state->status_bar_area.h + state->status_bar_area.border_thickness;
        state->editor_area.h = state->message_area.y;
        state->editor_area.w = state->window_w;

        //explorer area
//        if (state->file_explorer_area.y == 0)
        {
            state->file_explorer_area.y = ui_font_char_h;
        }
        state->file_explorer_area.w = state->window_w;
        state->file_explorer_area.h = state->message_area.y - state->file_explorer_area.y;
    }

    //Input buffers
    state->command_input.y = state->window_h - ui_font_char_h * 1.1f;
    state->text.x = state->editor_area.x + FILE_TEXT_LEFT_MARGIN;
    state->text.y = state->editor_area.y;

    state->camera_x = (state->camera_x / state->char_w) * state->char_w;
    state->camera_y = (state->camera_y / state->char_h) * state->char_h;

    //Command state buttons
    for (int i = 0; i < 10; i++)
    {
        Button* button = state->buttons + i;

        button->h = 0; //so that it is set to the size of the text
        
        Button_resize_text(button, state->ui_font);

        button->w = state->window_w;
        button->y = ui_font_char_h * i;
        button->font = state->ui_font;
    }

    //FIle explorer buttons
    for (int i = 0; i < state->file_count; i++)
    {
        Button* button = state->file_buttons + i;
        button->font = state->file_explorer_font;
        editor_position_file_button(state, button, i+1);
    }

    editor_update_file_explorer_camera(state);
}

bool editor_get_cursor_pos(ProgramState* state, int* out_x, int* out_y, int char_w, int char_h)
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return false;

    int startx = buffer->x;
    int starty = buffer->y;

    int cursor_x = startx;
    int cursor_y = starty;
    for (int i = 0; i < buffer->text.len; i++)
    {
        char c = buffer->text.text[i];

        if (c == '\n')
        {
            cursor_y += char_h;
            cursor_x = startx;
        }
        else
        {
            cursor_x += char_w;
        }
        
        if (i == buffer->cursor_index - 1)
        {
            break;
        }
        else if ((i == buffer->cursor_index) && (i == 0))
        {
            cursor_x = startx;
            cursor_y = starty;
            break;
        }
    }

    *out_x = cursor_x;
    *out_y = cursor_y;
    return true;
}


void editor_push_message(ProgramState* state, const String* msg)
{
    Queue_push(&(state->messages), msg);
}


void editor_push_text_action(ProgramState* state, const TextAction* new_action)
{
    Stack_push(&(state->undo_tree), new_action);
}


void editor_select_first_enabled_button(ProgramState* state, Button* buttons, int button_count)
{
    for (int i = 0; i < button_count; i++)
    {
        Button* button = buttons + i;
        if (button->state == BUTTON_STATE_ENABLED)
        {
            state->clicked_button = button;
            state->clicked_button->mouse_hovering = true;
            break;
        }
    }
}


void editor_undo_text_action(ProgramState* state, const TextAction* action)
{
    switch (action->type)
    {
        case TEXT_ACTION_WRITE:
        {
            if (action->text.text)
            {
                for (int i = action->text.len-1; i >= 0; i--) 
                {
                    String_remove(&(state->text.text), action->start_index + i, NULL);
                }
                editor_set_cursor(state, action->start_index);
            }
            else
            {
                String_remove(&(state->text.text), action->start_index, NULL); 
                editor_set_cursor(state, action->start_index);
            }
        } break;

        case TEXT_ACTION_REMOVE:
        {
            if (action->text.text)
            {
                for (int i = 0; i < action->text.len; i++)
                {
                    char c = action->text.text[i];
                    String_insert(&(state->text.text), c, action->start_index + i);
                }
                editor_set_cursor(state, action->start_index + (action->text.len-1));
            }
            else
            {
                String_insert(&(state->text.text), action->character, action->start_index);
                editor_set_cursor(state, action->start_index+1);
            }
        } break;
    } 

    Stack_push(&(state->redo_tree), action);
    free(action);
}


void editor_redo_text_action(ProgramState* state, const TextAction* action)
{
    switch (action->type)
    {
        case TEXT_ACTION_WRITE:
        {
            if (action->text.text)
            {
                String_insert_string(&(state->text.text), action->text.text,
                action->start_index);
            }
            else
            {
                String_insert(&(state->text.text), action->character, action->start_index);
                editor_set_cursor(state, action->start_index+1);
            }
        } break;

        case TEXT_ACTION_REMOVE:
        {
            if (action->text.text)
            {
                int end_index = (action->start_index + action->text.len) -1;
                for (int i = end_index; i >= action->start_index; i--)
                {
                    String_remove(&(state->text.text), i, NULL);
                }

                editor_set_cursor(state, action->start_index);
            }
            else
            {
                String_remove(&(state->text.text), action->start_index, NULL); 
                editor_set_cursor(state, action->start_index);
            }
        } break;
    }

    Stack_push(&(state->undo_tree), action);
    free(action);
}

void editor_update_file_explorer_camera(ProgramState* state)
{
    if (state->clicked_button == NULL) return;
    if (state->state != EDITOR_STATE_FILE_EXPLORER) return;

    int x = state->clicked_button->x - state->file_explorer_camera_x;
    int y = state->clicked_button->y - state->file_explorer_camera_y;
    int w = state->clicked_button->w;
    int h = state->clicked_button->h;
    int file_explorer_area_bottom = state->file_explorer_area.y + state->file_explorer_area.h;
    int margin_between_file_names = h * MARGIN_BETWEEN_FILE_NAMES_FACTOR;
    h += margin_between_file_names;


    if ((y + h) > file_explorer_area_bottom)
    {
        int diff = ((y + h) - file_explorer_area_bottom) / h;
        state->file_explorer_camera_y += (diff+1) * h;
    }

    if ((y < (state->file_explorer_area.y)))
    {
        int diff = (y - state->file_explorer_area.y) / h;
        state->file_explorer_camera_y += (diff-1) * h;
    }

    state->file_explorer_camera_y = (state->file_explorer_camera_y / h) * h;
} 









#include "editor.h"
#include "editor_event.h"
#include "editor_fileio.h"
#include "editor_input_buffer.h"

#include "button.h"
#include "draw.h"
#include <memory.h>
#include <string.h>
#include <SDL_syswm.h>

//everybody does these this is C shut up
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

const int CURSOR_BLINK_TIME = 1000;
const int MESSAGE_DURATION = 1000;


//NEXT OBJECTIVE:: DO ALL THE TEXT EDITING COOL SHIT WITH LCTRL AND COPY PASTING AND STUFF
//                 ALSO DISPLAYING MESSAGES TO THE USER IN THE BOTTOM RIGHT CORNER OR SOMETHING
//                 "FILE OPENED SUCCESSFULLY", "OPENING FILE FAILED", "PASTED XXX LINES"
//                 AND A STATUS BAR WITH THE FILENAME. CURRENT LINE AND CURRENT CHARACTER NUMBER


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
                                     SDL_WINDOW_SHOWN);
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
    
    state->font_size = 24;
    state->font = TTF_OpenFont("CONSOLA.ttf", state->font_size);
    state->static_font = TTF_OpenFont("CONSOLA.ttf", 20);
    if (!(state->font) || !(state->static_font))
    {
        printf("Loading Font Failed\nError Message: %s\n", TTF_GetError());
        return 4;
    }

    TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));

    state->editor_area_x = 0;
    state->editor_area_y = 0;
    state->editor_area_w = state->window_w;
 //   state->editor_area_h = state->window_h - state->char_h * 2.5f;

/*    {
        int char_h;
        TTF_SizeText(state->static_font, "A", NULL, &char_h);

        state->command_input.y = state->window_h - char_h*6;
        state->editor_area_h = state->window_h - char_h * 6.5f;

    }*/

    ButtonConfig config = {0};
    config.pressed_r = 110;
    config.pressed_g = 100;
    config.pressed_b = 100;
    config.font = state->font;
    config.h = state->char_h;

    {
        config.text = "Save";
        config.x = 0;
        config.y = config.h * 2;
        config.on_click = Button_save_on_click;
        config.on_input = Button_save_on_input;
        config.disabled = true;
        Button_init(state->buttons + 2, &config);
    }
    {
        config.text = "Open";
        config.x = 0;
        config.y = config.h;
        config.on_click = Button_save_on_click; //this is not an oversight.
        config.on_input = Button_open_on_input;
        config.disabled = true;
        Button_init(state->buttons + 1, &config);
    }
    {
        config.text = "File";
        config.y = 0;
        config.x = 0;
        config.on_click = Button_file_on_click;
        config.on_input = NULL;
        config.disabled = false;
        Button_init(state->buttons + 0, &config);

        Button_add_child(state->buttons + 0, state, 1);
        Button_add_child(state->buttons + 0, state, 2);
    }

    Queue_init(&(state->messages), sizeof(String));

    state->selection_start_index = -2;

    state->current_file = "CODE.c";
    editor_open_file(state);
    state->current_file = NULL;

    editor_resize_and_position_buttons(state);
}


void editor_destroy(ProgramState* state)
{
    TTF_CloseFont(state->font);
    TTF_Quit();
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}


void editor_loop(ProgramState* state)
{
    while (state->running)
    {
        editor_handle_events(state);
        editor_update(state);
        editor_draw(state);
    }
}


void editor_update(ProgramState* state)
{
    if (state->selection_start_index != -2)
    {
        state->draw_cursor = true;
    }
    else if ((SDL_GetTicks() - state->last_cursor_blink_tic) >= CURSOR_BLINK_TIME)
    {
        state->draw_cursor = !(state->draw_cursor);
        state->last_cursor_blink_tic = SDL_GetTicks();
    }

    if (state->message)
    {
        int tic = SDL_GetTicks();
        if (tic > (state->message_change_tic + MESSAGE_DURATION))
        {
            printf("Msg: %s\nChange tic: %d\nCurrent Tic: %d\nDuration: %d\n\n", state->message->text,
            state->message_change_tic, tic, MESSAGE_DURATION);
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


            int cursor_x = 0;
            int cursor_y = 0;
            editor_get_cursor_pos(state, &cursor_x, &cursor_y, state->char_w, state->char_h);
            cursor_x -= state->camera_x;
            cursor_y -= state->camera_y;

            if ((cursor_x + state->char_w) > state->editor_area_w)
            {
                state->camera_x += state->char_w;
            }
            if ((cursor_y + state->char_h) > state->editor_area_h)
            {
                state->camera_y += state->char_h;
            }

            if (cursor_y < state->editor_area_y)
            {
                state->camera_y -= state->char_h;
            }
            if (cursor_x < state->editor_area_x)
            {
                state->camera_x -= state->char_w;
            }

        } break;

        case EDITOR_STATE_COMMAND:
        {
            if (mouse_state)
            {
                state->clicked_button = NULL;
                for (int i = 0; i < 10; i++)
                {
                    Button* button = state->buttons + i;
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
        } break;
    }
}


void editor_draw(ProgramState* state)
{
    SDL_FillRect(state->window_surface, NULL, SDL_MapRGB(state->window_surface->format, 0, 0, 0)); //Clear

    {
        int char_h;
        TTF_SizeText(state->static_font, "A", NULL, &char_h);
        SDL_Rect border_line = 
        {
            0, state->editor_area_h,
            state->window_w, 4
        };
        SDL_FillRect(state->window_surface, &border_line, 0xbbbbbbff);
    }

    switch (state->state)
    {
        case EDITOR_STATE_COMMAND:
        {
            for (int i = 0; i < 10; i++)
            {
                Button_draw(state->buttons + i, state->font, state->window_surface);
            }
        } break;

        case EDITOR_STATE_COMMAND_INPUT:
        {
            editor_draw_input_buffer(state);
        } break;

        case EDITOR_STATE_EDIT:
        {
            editor_draw_input_buffer(state);
        } break;
    }

    if (state->state != EDITOR_STATE_COMMAND_INPUT)
    {
        if (!state->message)
        {
            state->message = Queue_pop(&(state->messages), true);
            state->message_change_tic = SDL_GetTicks();
        }
        if (state->message)
        {
            int char_h;
            TTF_SizeText(state->static_font, "A", NULL, &char_h);

            draw_text(state->static_font, state->window_surface, state->message->text,
                    0,
                    state->command_input.y,
                    255, 230, 230);
        }
    }
    SDL_UpdateWindowSurface(state->window);
}

void draw_text(TTF_Font* font, SDL_Surface* dst_surface, const char* text,
                int x, int y, int r, int g, int b)
{
    SDL_Color text_color = { r, g, b, 255 };
    SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, text_color);
    
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

    
    
    //don't blink while typing
    state->last_cursor_blink_tic = SDL_GetTicks();
    state->draw_cursor = true;
}



void editor_set_state(ProgramState* state, int new_state)
{
    switch (state->state)
    {
        case EDITOR_STATE_COMMAND_INPUT:
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
        } break;

        case EDITOR_STATE_COMMAND:
        {
            //reset to main buttons like File
            for (int i = 0; i < 10; i++)
            {
                Button* button = state->buttons + i;
                Button_disable_children(button, state);
            } 

            if (new_state != EDITOR_STATE_COMMAND_INPUT)
            {
                if (state->clicked_button)
                {
                    state->clicked_button->mouse_hovering = false;
                    state->clicked_button = NULL;
                }
            }
        } break;
    }

    state->state = new_state;
}


void editor_set_filename(ProgramState* state, const char* new_filename)
{
    state->current_file = new_filename;
}


void editor_resize_and_position_buttons(ProgramState* state)
{
    for (int i = 0; i < 10; i++)
    {
        Button* button = state->buttons + i;

        button->w = 0;
        button->h = 0; //so that it is set to the size of the text
        
        Button_resize_text(button, state->font);

        button->y = state->char_h * i;
    }

    state->editor_area_x = 0;
    state->editor_area_y = 0;
    state->editor_area_w = state->window_w;
    //state->editor_area_h = state->window_h - state->char_h * 2.5f;

    {
        int char_h;
        TTF_SizeText(state->static_font, "A", NULL, &char_h);

        state->command_input.y = state->window_h - char_h * 1.1f;
        state->editor_area_h = state->window_h - char_h * 4;
    }

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


void editor_push_message(ProgramState* state, String* msg)
{
    Queue_push(&(state->messages), msg);
}


void editor_select_first_enabled_button(ProgramState* state)
{
    int button_count = sizeof(state->buttons) / sizeof(Button);
    for (int i = 0; i < button_count; i++)
    {
        Button* button = state->buttons + i;
        if (button->state == BUTTON_STATE_ENABLED)
        {
            state->clicked_button = button;
        }
    }
}
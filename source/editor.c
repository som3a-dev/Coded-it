#include "editor.h"
#include "button.h"
#include <memory.h>
#include <string.h>



const int CURSOR_BLINK_TIME = 1000;


int editor_init(ProgramState* state)
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
    
    state->window = SDL_CreateWindow("Coded-it", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
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
    
    state->font = TTF_OpenFont("CONSOLA.ttf", 24);
    if (!state->font)
    {
        printf("Loading Font Failed\nError Message: %s\n", TTF_GetError());
        return 4;
    }

    {
        ButtonConfig config = {0};
        config.pressed_r = 50;
        config.pressed_g = 30;
        config.pressed_b = 30;
        config.text = "Save";
        config.font = state->font;
        config.on_click = Button_save_on_click;
        config.on_input = Button_save_on_input;
        Button_init(state->buttons + 0, &config);
    }

    state->current_file = "test.txt";
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


void editor_handle_events(ProgramState* state)
{
    SDL_Event e;
    
    SDL_WaitEventTimeout(&e, 1000);
    
    switch (e.type)
    {
        case SDL_QUIT:
        {
            state->running = false;
        } break;
        
        case SDL_TEXTINPUT:
        {
            InputBuffer* buffer = editor_get_current_input_buffer(state);
 
            String_insert(&(buffer->text), e.text.text[0], buffer->cursor_index);
            //system("@cls||clear");
            //printf("%s\n", state->text);
            
            editor_set_cursor(state, buffer->cursor_index + 1);
        } break;

        case SDL_MOUSEMOTION:
        {
            for (int i = 0; i < 10; i++)
            {
                Button_on_mouse_move(state->buttons + i, e.motion.x, e.motion.y);
            }
        } break;
        
        case SDL_KEYDOWN:
        {
            InputBuffer* buffer = editor_get_current_input_buffer(state);
            
            switch (e.key.keysym.sym)
            {
                case SDLK_BACKSPACE:
                {
                    //String_pop(&(state->text));
                    String_remove(&(buffer->text), buffer->cursor_index - 1);
                    
                    editor_set_cursor(state, buffer->cursor_index - 1);
                } break;
                
                case SDLK_RETURN:
                {
                    //String_push(&(state->text), '\n');
                    switch (state->state)
                    {
                        case EDITOR_STATE_EDIT:
                        {
                            String_insert(&(buffer->text), '\n', buffer->cursor_index);
                            editor_set_cursor(state, buffer->cursor_index + 1);
                        } break;

                        case EDITOR_STATE_COMMAND_INPUT:
                        {
                            editor_set_state(state, EDITOR_STATE_EDIT);
                        } break;
                    }
                } break;
                
                case SDLK_UP:
                {
                    int prev_newline = String_get_previous_newline(&(buffer->text),
                    buffer->cursor_index);
                    if (prev_newline == -1)
                    {
                        return;
                    }
                    
                    int cursor_index_in_line = buffer->cursor_index - prev_newline - 1;
                    printf("Cursor index in line: %d\n", cursor_index_in_line);
                    
                    int newline_before_prev_newline = String_get_previous_newline(buffer,
                    prev_newline);
                    
                    //cap cursor_index_in_line at the length of the previous line - 1
                    int prev_line_len = prev_newline - newline_before_prev_newline;
                    if (cursor_index_in_line >= prev_line_len)
                    {
                        cursor_index_in_line = prev_line_len - 1;
                    }
                    
                    editor_set_cursor(state,
                    newline_before_prev_newline + cursor_index_in_line + 1);
                } break;
                
                case SDLK_DOWN:
                {
                    int prev_newline = String_get_previous_newline(&(buffer->text),
                    buffer->cursor_index);
                    
                    int cursor_index_in_line = buffer->cursor_index - prev_newline - 1;
                    //printf("Cursor index in line: %d\n", cursor_index_in_line);
                    
                    int next_newline = String_get_next_newline(&(buffer->text),
                    prev_newline);
                    //printf("Next newline: %d\n", next_newline);
                    
                    if (next_newline == buffer->text.len)
                    {
                        //We are at the last line
                        break;
                    }
                    
                    int next_next_newline = String_get_next_newline(&(buffer->text),
                    next_newline);
                    //printf("Next newline: %d\n", next_next_newline);
                    
                    //cap cursor_index_in_line at the length of the previous line - 1
                    int next_line_len = next_next_newline - next_newline - 1;
                    if (next_line_len < 0) next_line_len = 0;
                    //printf("Next line len: %d\n", next_line_len);
                    
                    if (cursor_index_in_line > next_line_len)
                    {
                        cursor_index_in_line = next_line_len;
                    }
                    
                    editor_set_cursor(state, next_newline + cursor_index_in_line + 1);
                } break;
                
                case SDLK_LEFT:
                {
                    editor_set_cursor(state, buffer->cursor_index - 1);
                } break;
                
                case SDLK_RIGHT:
                {
                    editor_set_cursor(state, buffer->cursor_index + 1);
                } break;
                
                case SDLK_TAB:
                {
                    String_insert(&(buffer->text), ' ', buffer->cursor_index);
                    editor_set_cursor(state, buffer->cursor_index+1);
                    String_insert(&(buffer->text), ' ', buffer->cursor_index);
                    editor_set_cursor(state, buffer->cursor_index+1);
                } break;

                case SDLK_LCTRL:
                {
                    int new_state = state->state + 1;

                    if (new_state >= EDITOR_STATE_COUNT)
                    {
                        new_state = EDITOR_STATE_EDIT; //TODO(omar): maybe we should set to zero instead.
                    }

                    editor_set_state(state, new_state);
                } break;

                case SDLK_INSERT:
                {
                    editor_save_file(state);
                }
            }
        } break;
    }
}


void editor_update(ProgramState* state)
{
    if ((SDL_GetTicks() - state->last_cursor_blink_tic) >= CURSOR_BLINK_TIME)
    {
        state->draw_cursor = !(state->draw_cursor);
        state->last_cursor_blink_tic = SDL_GetTicks();
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
        } break;

        case EDITOR_STATE_COMMAND:
        {
            if (mouse_state)
            {
                state->clicked_button = NULL;
                for (int i = 0; i < 10; i++)
                {
                    Button* button = state->buttons + i;
                    if (Button_is_mouse_hovering(button))
                    {
                        //do
                        if (button->on_click)
                        {
                            button->on_click(state);
                            state->clicked_button = button;
                            break;
                        }
                    }
                }
            }
        } break;
    }
}


void editor_draw(ProgramState* state)
{
    //Update the size of a character
    TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));
    
    SDL_FillRect(state->window_surface, NULL, SDL_MapRGB(state->window_surface->format, 0, 0, 0)); //Clear
    
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
            editor_draw_input_buffer(state, 0, 600-state->char_h*2);
        } break;
        case EDITOR_STATE_EDIT:
        {
            editor_draw_input_buffer(state, 0, 0);
        } break;
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


void editor_save_file(const ProgramState* state)
{
    FILE* fp;
    fopen_s(&fp, state->current_file, "w");
    
    if (!fp)
    {
        //TODO(omar): tell the fucking user the filename is invalid
        printf("Couldn't open file '%s'.\n", state->current_file);
        return;
    }

    //fucking text.text.text
    fwrite(state->text.text.text, sizeof(char), state->text.text.len, fp);

    fclose(fp);
}


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


void editor_draw_input_buffer(ProgramState* state, int startx, int starty)
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    if (!buffer) return;

    if (state->draw_cursor)
    {
        //TODO(omar): i don't like having 2 loops one to draw the text and one to draw the cursor.
        //Try to find a better way
        int cursor_x = startx;
        int cursor_y = starty;
        for (int i = 0; i < buffer->text.len; i++)
        {
            char c = buffer->text.text[i];
            if (c == '\n')
            {
                cursor_y += state->char_h;
                cursor_x = startx;
            }
            else
            {
                cursor_x += state->char_w;
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
        SDL_Rect cursor_rect = { cursor_x, cursor_y, state->char_w, state->char_h};
        SDL_FillRect(state->window_surface, &cursor_rect, SDL_MapRGB(state->window_surface->format, 200, 200, 200));
    }
    
    int x = startx;
    int y = starty;
    for (int i = 0; i < buffer->text.len; i++)
    {
        bool draw_char = true;
        
        char c = buffer->text.text[i];
        if (c == '\n')
        {
            y += state->char_h;
            x = startx;
            draw_char = false;
        }
        
        if (draw_char)
        {
            char str[2] = { c, '\0' };
            draw_text(state->font, state->window_surface, str, x, y, 255, 255, 255);
            x += state->char_w;
        }
    }
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
                    state->clicked_button->on_input(state, &(buffer->text));
                }
            }
            String_clear(&(buffer->text));
            memset(buffer, 0, sizeof(InputBuffer));
        } break;
    }

    state->state = new_state;
}


void editor_set_filename(ProgramState* state, const char* new_filename)
{
    state->current_file = new_filename;
}
#include "editor.h"
#include "button.h"
#include <memory.h>



const int CURSOR_BLINK_TIME = 1000;


//NEXT OBJECTIVE: STATE MACHINE PROGRAM. SWITCH BETWEEN EDIT STATE AND COMMAND STATE WITH CTRL-O

int editor_init(ProgramState* state)
{
    state->running = true;
    state->text.text = NULL;
    state->text.len = 0;
    state->draw_cursor = true;
    state->last_cursor_blink_tic = 0;
    state->cursor_index = 0;
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

    memset(state->buttons, 0, sizeof(Button) * 10);

    Button_init(state->buttons + 0, BUTTON_STATE_ENABLED, 400, 400, 64, 64,
    120, 30, 20,
    200, 30, 20,
    "Text");
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
            String_insert(&(state->text), e.text.text[0], state->cursor_index);
            //system("@cls||clear");
            //printf("%s\n", state->text);
            
            editor_set_cursor(state, state->cursor_index + 1);
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
            switch (e.key.keysym.sym)
            {
                case SDLK_BACKSPACE:
                {
                    //String_pop(&(state->text));
                    String_remove(&(state->text), state->cursor_index - 1);
                    
                    editor_set_cursor(state, state->cursor_index - 1);
                } break;
                
                case SDLK_RETURN:
                {
                    //String_push(&(state->text), '\n');
                    String_insert(&(state->text), '\n', state->cursor_index);
                    
                    editor_set_cursor(state, state->cursor_index + 1);
                } break;
                
                case SDLK_UP:
                {
                    int prev_newline = String_get_previous_newline(&(state->text), state->cursor_index);
                    if (prev_newline == -1)
                    {
                        return;
                    }
                    
                    int cursor_index_in_line = state->cursor_index - prev_newline - 1;
                    printf("Cursor index in line: %d\n", cursor_index_in_line);
                    
                    int newline_before_prev_newline = String_get_previous_newline(&(state->text), prev_newline);
                    
                    //cap cursor_index_in_line at the length of the previous line - 1
                    int prev_line_len = prev_newline - newline_before_prev_newline;
                    if (cursor_index_in_line >= prev_line_len)
                    {
                        cursor_index_in_line = prev_line_len - 1;
                    }
                    
                    editor_set_cursor(state, newline_before_prev_newline + cursor_index_in_line + 1);
                } break;
                
                case SDLK_DOWN:
                {
                    int prev_newline = String_get_previous_newline(&(state->text), state->cursor_index);
                    
                    int cursor_index_in_line = state->cursor_index - prev_newline - 1;
                    //printf("Cursor index in line: %d\n", cursor_index_in_line);
                    
                    int next_newline = String_get_next_newline(&(state->text), prev_newline);
                    //printf("Next newline: %d\n", next_newline);
                    
                    if (next_newline == state->text.len)
                    {
                        //We are at the last line
                        break;
                    }
                    
                    int next_next_newline = String_get_next_newline(&(state->text), next_newline);
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
                    editor_set_cursor(state, state->cursor_index - 1);
                } break;
                
                case SDLK_RIGHT:
                {
                    editor_set_cursor(state, state->cursor_index + 1);
                } break;
                
                case SDLK_TAB:
                {
                    String_insert(&(state->text), ' ', state->cursor_index);
                    editor_set_cursor(state, state->cursor_index+1);
                    String_insert(&(state->text), ' ', state->cursor_index);
                    editor_set_cursor(state, state->cursor_index+1);
                } break;

                case SDLK_LCTRL:
                {
                    state->state++;

                    if (state->state >= EDITOR_STATE_COUNT)
                    {
                        state->state = EDITOR_STATE_EDIT; //TODO(omar): maybe we should set to zero instead.
                    }
                } break;
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
                if (state->text.len != 0)
                {
                    int mouse_char_x = mouse_x / state->char_w;
                    int mouse_char_y = mouse_y / state->char_h;

                    int char_x = 0;
                    int char_y = 0;
                    for (int i = 0; i <= state->text.len; i++)
                    {
                        if ((mouse_char_x == char_x) && (mouse_char_y == char_y))
                        {
                            editor_set_cursor(state, i);
                            break;
                        }

                        char_x++;

                        if (state->text.text[i] == '\n')
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
                for (int i = 0; i < 10; i++)
                {
                    Button_on_mouse_click(state->buttons + i, mouse_state);
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

        case EDITOR_STATE_EDIT:
        {
            if (state->draw_cursor)
            {
                //TODO(omar): i don't like having 2 loops one to draw the text and one to draw the cursor.
                //Try to find a better way
                int cursor_x = 0;
                int cursor_y = 0;
                for (int i = 0; i < state->text.len; i++)
                {
                    char c = state->text.text[i];
                    if (c == '\n')
                    {
                        cursor_y += state->char_h;
                        cursor_x = 0;
                    }
                    else
                    {
                        cursor_x += state->char_w;
                    }
                    
                    if (i == state->cursor_index - 1)
                    {
                        break;
                    }
                    else if ((i == state->cursor_index) && (i == 0))
                    {
                        cursor_x = 0;
                        cursor_y = 0;
                        break;
                    }
                }
                SDL_Rect cursor_rect = { cursor_x, cursor_y, state->char_w, state->char_h};
                SDL_FillRect(state->window_surface, &cursor_rect, SDL_MapRGB(state->window_surface->format, 200, 200, 200));
            }
            
            int x = 0;
            int y = 0;
            for (int i = 0; i < state->text.len; i++)
            {
                bool draw_char = true;
                
                char c = state->text.text[i];
                if (c == '\n')
                {
                    y += state->char_h;
                    x = 0;
                    draw_char = false;
                }
                
                if (draw_char)
                {
                    char str[2] = { c, '\0' };
                    draw_text(state->font, state->window_surface, str, x, y, 255, 255, 255);
                    x += state->char_w;
                }
            }
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
    state->cursor_index = index;
    if (state->cursor_index < 0)
    {
        state->cursor_index = 0;
    }
    if (state->cursor_index > state->text.len)
    {
        state->cursor_index = state->text.len;
    }
    
    //don't blink while typing
    state->last_cursor_blink_tic = SDL_GetTicks();
    state->draw_cursor = true;
}

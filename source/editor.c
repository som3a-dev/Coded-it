#include "editor.h"

const int CURSOR_BLINK_TIME = 1000;


void editor_init(ProgramState* state)
{
    state->running = true;
    state->text.text = NULL;
    state->text.len = 0;
    state->draw_cursor = true;
    state->last_cursor_blink_tic = 0;
    state->cursor_index = 0;

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
        system("@cls||clear");
        printf("%s\n", state->text);

        //don't blink while typing
        state->last_cursor_blink_tic = SDL_GetTicks();
        state->draw_cursor = true;
        state->cursor_index++;
    } break;

    case SDL_KEYDOWN:
    {
        switch (e.key.keysym.sym)
        {
        case SDLK_BACKSPACE:
        {
            //String_pop(&(state->text));
            String_remove(&(state->text), state->cursor_index - 1);

            if (state->cursor_index > 0)
            {
                state->last_cursor_blink_tic = SDL_GetTicks();
                state->draw_cursor = true;
                state->cursor_index--;
            }
        } break;

        case SDLK_RETURN:
        {
            //String_push(&(state->text), '\n');
            String_insert(&(state->text), '\n', state->cursor_index);

            state->last_cursor_blink_tic = SDL_GetTicks();
            state->draw_cursor = true;
            state->cursor_index++;
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

            state->cursor_index = newline_before_prev_newline + cursor_index_in_line + 1;

            state->last_cursor_blink_tic = SDL_GetTicks();
            state->draw_cursor = true;
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

            state->cursor_index = next_newline + cursor_index_in_line + 1;

            state->last_cursor_blink_tic = SDL_GetTicks();
            state->draw_cursor = true;
        } break;

        case SDLK_LEFT:
        {
            if (state->cursor_index > 0)
            {
                state->cursor_index--;
                state->last_cursor_blink_tic = SDL_GetTicks();
                state->draw_cursor = true;
            }
        } break;

        case SDLK_RIGHT:
        {
            state->cursor_index++;
            state->last_cursor_blink_tic = SDL_GetTicks();
            state->draw_cursor = true;
        } break;

        case SDLK_TAB:
        {
            String_insert(&(state->text), ' ', state->cursor_index);
            state->cursor_index++;
            String_insert(&(state->text), ' ', state->cursor_index);
            state->cursor_index++;
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
}


void editor_draw(ProgramState* state)
{
    //Update the size of a character
    TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));

    SDL_FillRect(state->window_surface, NULL, SDL_MapRGB(state->window_surface->format, 60, 60, 60)); //Clear

    //draw_text(state, state->text.text, 36 / 2, 0, 255, 255, 255);

    int x = 0;
    int y = 0;
    int cursor_x = 0;
    int cursor_y = 0;
    for (int i = 0; i < state->text.len; i++)
    {
        bool draw_char = true;

        char c = state->text.text[i];
        if (c == '\n')
        {
            //char str[2] = { '|', '\0'};
            //draw_text(state, str, x, y, 255, 255, 255);

            y += state->char_h;
            //cursor_y += state->char_h;
            x = 0;
            draw_char = false;
        }

        if (i == state->cursor_index - 1)
        {
            cursor_x = x;
            cursor_y = y;

            if (draw_char)
            {
                cursor_x += state->char_w;
            }
        }

        if (draw_char)
        {
            char str[2] = { c, '\0' };
            editor_draw_text(state, str, x, y, 255, 255, 255);
            x += state->char_w;
        }
    }

    if (state->draw_cursor)
    {
        SDL_Rect cursor_rect = { cursor_x, cursor_y + 2, state->char_w, state->char_h - 2 };
        SDL_FillRect(state->window_surface, &cursor_rect, SDL_MapRGB(state->window_surface->format, 200, 200, 200));
    }

    SDL_UpdateWindowSurface(state->window);
}


void editor_draw_text(ProgramState* state, const char* text, int x, int y, int r, int g, int b)
{
    SDL_Color text_color = { r, g, b, 255 };
    SDL_Surface* text_surface = TTF_RenderText_Solid(state->font, text, text_color);

    SDL_Rect text_dst = { x, y, 0, 0 };
    TTF_SizeText(state->font, text, &(text_dst.w), &(text_dst.h));

    SDL_BlitSurface(text_surface, NULL, state->window_surface, &text_dst);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "string.h"


typedef struct
{
    SDL_Window* window;
    SDL_Surface* window_surface;
    TTF_Font* font;
    bool running;
    
    String text;
    
    int char_w;
    int char_h;
    
    bool draw_cursor; //used for a blinking cursor
    int last_cursor_blink_tic;
} ProgramState;


const int CURSOR_BLINK_TIME = 1000; //cursor blinks every CURSOR_BLINK_TIME seconds


void loop(ProgramState* state);
void handle_events(ProgramState* state);
void update(ProgramState* state);
void draw(ProgramState* state);

void draw_text(ProgramState* state, const char* text, int x, int y, int r, int g, int b);


int main(int argc, char** argv)
{
    ProgramState state = {0};
    state.running = true;
    state.text.text = NULL;
    state.text.len = 0;
    state.draw_cursor = true;
    state.last_cursor_blink_tic = 0;
    
    const char* error = NULL;
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    error = SDL_GetError();
    if (error[0])
    {
        printf("Initializing SDL failed!\nError Message: '%s'\n", error);
        return 1;
    }
    
    state.window = SDL_CreateWindow( "Coded-it", 100, 100, 800, 600, SDL_WINDOW_SHOWN );
    if (!state.window)
    {
        printf("SDL window creation failed!\n");
        return 2;
    }
    state.window_surface = SDL_GetWindowSurface(state.window);
    
    TTF_Init();
    error = TTF_GetError();
    if (error[0])
    {
        printf("Initializing SDL_ttf failed!\nError Message: '%s'\n", error);
        return 3;
    }
    
    state.font = TTF_OpenFont("CONSOLA.ttf", 36);
    if (!state.font)
    {
        printf("Loading Font Failed\nError Message: %s\n", TTF_GetError());
        return 4;
    }
    
    loop(&state);
    
    TTF_CloseFont(state.font);
    TTF_Quit();
    SDL_DestroyWindow(state.window);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}


void loop(ProgramState* state)
{      
    while (state->running)
    {
        handle_events(state);
        update(state);
        draw(state);
    }
}


void handle_events(ProgramState* state)
{
    SDL_Event e;
    
    SDL_WaitEventTimeout(&e, 100);
    
    switch (e.type)
    {
        case SDL_QUIT:
        {
            state->running = false;
        } break;
        
        case SDL_TEXTINPUT:
        {
            String_push(&(state->text), e.text.text[0]);
            system("@cls||clear");
            printf("%s\n", state->text);
            
            //don't blink while typing
            state->last_cursor_blink_tic = SDL_GetTicks();
            state->draw_cursor = true;
        } break;
        
        case SDL_KEYDOWN:
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_BACKSPACE:
                {
                    String_pop(&(state->text));
                    
                    state->last_cursor_blink_tic = SDL_GetTicks();
                    state->draw_cursor = true;
                } break;
                
                case SDLK_RETURN:
                {
                    String_push(&(state->text), '\n');
                    
                    state->last_cursor_blink_tic = SDL_GetTicks();
                    state->draw_cursor = true;
                } break;
            }
        } break;
    }
}


void update(ProgramState* state)
{
    if ((SDL_GetTicks() - state->last_cursor_blink_tic) >= CURSOR_BLINK_TIME)
    {
        state->draw_cursor = !(state->draw_cursor);
        state->last_cursor_blink_tic = SDL_GetTicks();
    }
}


void draw(ProgramState* state)
{
    //Update the size of a character
    TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));
    
    SDL_FillRect(state->window_surface, NULL, SDL_MapRGB(state->window_surface->format, 60, 60, 60)); //Clear
    
    //draw_text(state, state->text.text, 36 / 2, 0, 255, 255, 255);
    
    int x = 0;
    int y = 0;
    for (int i = 0; i < state->text.len; i++)
    {
        char c = state->text.text[i];
        if (c == '\n')
        {
            y += state->char_h;
            x = 0;
            continue;
        }
        
        char str[2] = {c, '\0'};
        draw_text(state, str, x, y, 255, 255, 255);
        x += state->char_w;
    }
    
    if (state->draw_cursor)
    {
        SDL_Rect cursor_rect = {x, y + 2, state->char_w, state->char_h - 2};
        SDL_FillRect(state->window_surface, &cursor_rect, SDL_MapRGB(state->window_surface->format, 200, 200, 200));
    }
    
    SDL_UpdateWindowSurface(state->window);
}


void draw_text(ProgramState* state, const char* text, int x, int y, int r, int g, int b)
{
    SDL_Color text_color = {r, g, b, 255};
    SDL_Surface* text_surface = TTF_RenderText_Solid(state->font, text, text_color);
    
    SDL_Rect text_dst = {x, y, 0, 0};
    TTF_SizeText(state->font, text, &(text_dst.w), &(text_dst.h));
    
    SDL_BlitSurface(text_surface, NULL, state->window_surface, &text_dst);
}

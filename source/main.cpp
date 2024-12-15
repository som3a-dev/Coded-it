#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>


struct ProgramState
{
    SDL_Window* window = NULL;
    SDL_Surface* window_surface = NULL;
    TTF_Font* font = NULL;
    bool running = true;
};


void loop(ProgramState* state);
void handle_events(ProgramState* state);
void draw(ProgramState* state);


int main(int argc, char** argv)
{
    ProgramState state;
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
        draw(state);
    }
}


void handle_events(ProgramState* state)
{
    SDL_Event e;
    SDL_WaitEvent(&e);

    switch (e.type)
    {
        case SDL_QUIT:
        {
            state->running = false;
        } break;
    }
}


void draw(ProgramState* state)
{
    SDL_FillRect(state->window_surface, NULL, SDL_MapRGB(state->window_surface->format, 60, 60, 60)); //Clear

    {
        const char* text = "Testing Font";
        SDL_Color text_color = {255, 255, 255, 255};
        SDL_Surface* text_surface = TTF_RenderText_Solid(state->font, text, text_color);
        
        SDL_Rect text_dst = {0, 0, 0, 0};
        TTF_SizeText(state->font, text, &(text_dst.w), &(text_dst.h));
        
        SDL_BlitSurface(text_surface, NULL, state->window_surface, &text_dst);
    }
    
    SDL_UpdateWindowSurface(state->window);
}

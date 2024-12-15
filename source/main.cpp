#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

static SDL_Window* window = NULL;
static bool running = true;


void loop();


int main(int argc, char** argv)
{
    const char* error = NULL;
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    error = SDL_GetError();
    if (error[0])
    {
        printf("Initializing SDL failed!\nError Message: '%s'\n", error);
    }

    window = SDL_CreateWindow( "Coded-it", 100, 100, 800, 600, SDL_WINDOW_SHOWN );
    if (!window)
    {
        printf("SDL window creation failed!\n");
        return 1;
    }

    TTF_Init();
    error = TTF_GetError();
    if (error[0])
    {
        printf("Initializing SDL_ttf failed!\nError Message: '%s'\n", error);
    }

    loop();

    TTF_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}


void loop()
{
    TTF_Font* font = TTF_OpenFont("CONSOLA.ttf", 36);
    if (!font)
    {
        printf("Loading Font Failed\nError Message: %s\n", TTF_GetError());
    }
    
    SDL_Surface* win_surface = SDL_GetWindowSurface(window);
    while (running)
    {
        SDL_Event e;
        SDL_WaitEvent(&e);

        switch (e.type)
        {
            case SDL_QUIT:
            {
                running = false;
            } break;
        }

        //Draw
        SDL_FillRect(win_surface, NULL, SDL_MapRGB(win_surface->format, 60, 60, 60)); //Clear

        const char* text = "Testing Font";
        SDL_Color text_color = {255, 255, 255, 255};
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, text_color);
        
        SDL_Rect text_dst = {0, 0, 0, 0};
        TTF_SizeText(font, text, &(text_dst.w), &(text_dst.h));
        
        SDL_BlitSurface(text_surface, NULL, win_surface, &text_dst);
        
        SDL_UpdateWindowSurface(window);
    }
}

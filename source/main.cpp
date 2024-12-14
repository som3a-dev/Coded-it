#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

static SDL_Window* window = NULL;
static bool running = true;


void loop();


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    window = SDL_CreateWindow( "Coded-it", 100, 100, 800, 600, SDL_WINDOW_SHOWN );
    if (!window)
    {
        printf("SDL window creation failed!\n");
        return 1;
    }

    loop();
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}


void loop()
{
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
        SDL_FillRect(win_surface, NULL, SDL_MapRGB(win_surface->format, 110, 110, 110));
        SDL_UpdateWindowSurface(window);
    }
}

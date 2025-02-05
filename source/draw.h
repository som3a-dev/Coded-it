#include <SDL.h>


//NOTE(omar): yes this file is not used for now.
//            i initially began writing it to draw lines but then realized
//            i could just use SDL_FillRect to draw lines...


void draw_pixel(SDL_Surface* surface,
                int x, int y,
                uint8_t r, uint8_t g, uint8_t b);


#include "draw.h"


//NOTE(omar): if you face slowdowns. optimize this function
void draw_pixel(SDL_Surface* surface,
                int x, int y,
                uint8_t r, uint8_t g, uint8_t b)
{
    int w = surface->w;
    int h = surface->h;

    if ((x > w) || (y > h))
    {
        return;
    }
    if ((x < 0) || (y < 0))
    {
        return;
    }

    int pixel_index = (x + (y * w));
    int pixel_index_in_bytes = pixel_index * surface->format->BytesPerPixel;

    uint32_t color = 0;
    color |= ((uint32_t)b) << surface->format->Bshift;
    color |= ((uint32_t)g) << surface->format->Gshift;
    color |= ((uint32_t)r) << surface->format->Rshift;


    uint32_t* pixel = (char*)(surface->pixels) + pixel_index_in_bytes;
    *pixel = color;
}

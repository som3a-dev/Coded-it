#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"


int ulen_helper(unsigned x)
{
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}


bool SDL_is_ctrl_pressed(uint8_t* keystate)
{
    return keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL];
}


void SDL_FillRectAlpha(SDL_Surface* surface, const SDL_Rect* rect, SDL_Color* color)
{
    uint8_t r = color->r;
    uint8_t g = color->g; 
    uint8_t b = color->b;
    uint8_t a = color->a;

    int right = rect->x + rect->w;
    int bottom = rect->y + rect->h;
    for (int x = rect->x; x < right; x++)
    {
        if (x < 0) continue;
        if (x > surface->w) goto end;
        for (int y = rect->y; y < bottom; y++)
        {
            if (y < 0) continue;
            if (y > surface->h) goto end;

            Uint32* pixel = ((Uint32*)(surface->pixels)) + x + (y * surface->w);
            Uint32 dst_color = *pixel;

            uint8_t dst_r = (dst_color >> surface->format->Rshift) & 0xff;
            uint8_t dst_g = (dst_color >> surface->format->Gshift) & 0xff;
            uint8_t dst_b = (dst_color >> surface->format->Bshift) & 0xff;

            dst_r = (dst_r * (255 - a) + r * (a)) / 255;
            dst_g = (dst_g * (255 - a) + g * (a)) / 255;
            dst_b = (dst_b * (255 - a) + b * (a)) / 255;
//            dst_g = (dst_g * (1 - a) + g * (a));
//            dst_b = (dst_b * (1 - a) + b * (a));

            dst_color = (dst_r << surface->format->Rshift) | (dst_g << surface->format->Gshift) | (dst_b << surface->format->Bshift);

            *pixel = dst_color;
        }
    }

    end:
    return;
}


bool rgb_hex_str_to_int(const char* str, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a)
{
    if (strlen(str) < 6)
    {
        return false;
    }

    char rstr[3] = {str[0], str[1], 0};
    char gstr[3] = {str[2], str[3], 0};
    char bstr[3] = {str[4], str[5], 0};
    
    if (r)
    {
        *r = strtol(rstr, NULL, 16);
    }
    if (g)
    {
        *g = strtol(gstr, NULL, 16);
    }
    if (b)
    {
        *b = strtol(bstr, NULL, 16);
    }
    if (a)
    {
        if (strlen(str) >= 8)
        {
            char astr[3] = {str[6], str[7], 0};
            *a = strtol(astr, NULL, 16);
        }
        else
        {
            *a = 255;
        }
    }

    return true;
}
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
    if ((a) && (strlen(str) >= 8))
    {
        char astr[3] = {str[6], str[7], 0};
        *a = strtol(astr, NULL, 16);
    }

    return true;
}
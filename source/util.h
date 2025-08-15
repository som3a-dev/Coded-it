#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

int ulen_helper(unsigned x);

bool SDL_is_ctrl_pressed(uint8_t* keystate);

void SDL_FillRectAlpha(SDL_Surface* surface, const SDL_Rect* rect, SDL_Color* color);

bool rgb_hex_str_to_int(const char* str, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
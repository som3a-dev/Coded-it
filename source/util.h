#pragma once

#include <stdbool.h>
#include <stdint.h>

int ulen_helper(unsigned x);

bool SDL_is_ctrl_pressed(uint8_t* keystate);

bool rgb_hex_str_to_int(const char* str, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
#pragma once


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
    
    int cursor_index; //the index of the character the cursor is on.
    bool draw_cursor; //used for a blinking cursor
    int last_cursor_blink_tic;
} ProgramState;


void editor_init(ProgramState* state);
void editor_destroy(ProgramState* state);

void editor_loop(ProgramState* state);

void editor_handle_events(ProgramState* state);
void editor_update(ProgramState* state);
void editor_draw(ProgramState* state);

void editor_draw_text(ProgramState* state, const char* text, int x, int y, int r, int g, int b);

void editor_set_cursor(ProgramState* state, int index);
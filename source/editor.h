#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "string.h"
#include "button.h"


enum
{
    EDITOR_STATE_EDIT,
    EDITOR_STATE_COMMAND,
    EDITOR_STATE_COMMAND_INPUT,
    EDITOR_STATE_COUNT //not an actual state. used for counting
};


typedef struct
{
    String text;
    int cursor_index;
} InputBuffer;

typedef struct _ProgramState
{
    SDL_Window* window;
    SDL_Surface* window_surface;
    TTF_Font* font;
    bool running;

    int state;

    const char* current_file;
    
    InputBuffer text;
    InputBuffer command_input;
    Button* clicked_button; //the button clicked during the last
                            //EDITOR_STATE_COMMAND if any

//    String command_input_result; //the text the user sent/typed to us from
                                 //EDITOR_STATE_COMMAND_INPUT to be used by the
                                 //clicked button from EDITOR_STATE_COMMAND

    int char_w;
    int char_h;
    
    bool draw_cursor; //used for a blinking cursor
    int last_cursor_blink_tic;

    Button buttons[10];
} ProgramState;


int editor_init(ProgramState* state);
void editor_destroy(ProgramState* state);

void editor_loop(ProgramState* state);

void editor_handle_events(ProgramState* state);
void editor_update(ProgramState* state);
void editor_draw(ProgramState* state);
void editor_draw_input_buffer(ProgramState* state,
                              int startx, int starty);

//TODO(omar): move this function from editor.h/.c into another more suitable file
void draw_text(TTF_Font* font, SDL_Surface* dst_surface, const char* text,
                int x, int y, int r, int g, int b);

void editor_set_cursor(ProgramState* state, int index);

void editor_save_file(const ProgramState* state);

InputBuffer* editor_get_current_input_buffer(const ProgramState* state);

void editor_set_state(ProgramState* state, int new_state);

void editor_set_filename(ProgramState* state, const char* new_filename);
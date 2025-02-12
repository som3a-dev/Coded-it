#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "string.h"
#include "button.h"
#include "queue.h"

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

    int x;
    int y;
} InputBuffer;


typedef struct _ProgramState
{
    SDL_Window* window;
    SDL_Surface* window_surface;
    TTF_Font* font;
    int font_size;
    bool running;

    int window_w;
    int window_h;

    //NOTE(omar): this is the offset at which we render the
    //            file's text.
    //            used for scrolling.
    //            you can thank my gamedev background for it
    //            being named camera
    int camera_x;
    int camera_y;

    int editor_area_x;
    int editor_area_y;
    int editor_area_w;
    int editor_area_h;

    int state;

    const char* current_file;

    Queue messages;
    String* message;
    uint32_t message_change_tic;
    
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
void editor_draw_input_buffer(ProgramState* state);

//TODO(omar): move this function from editor.h/.c into another more suitable file
void draw_text(TTF_Font* font, SDL_Surface* dst_surface, const char* text,
                int x, int y, int r, int g, int b);

void editor_set_cursor(ProgramState* state, int index);

void editor_save_file(const ProgramState* state);

void editor_open_file(ProgramState* state);

InputBuffer* editor_get_current_input_buffer(const ProgramState* state);

void editor_set_state(ProgramState* state, int new_state);

void editor_set_filename(ProgramState* state, const char* new_filename);

void editor_resize_and_position_buttons(ProgramState* state); //called after
                                                              //changing font size

//gets position in X and Y of the current input buffer's cursor
//returns false if there is no current input buffer.
bool editor_get_cursor_pos(ProgramState* state, int* out_x, int* out_y);

void editor_push_message(ProgramState* state, String* msg);
#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "string.h"
#include "array.h"


enum
{
    BUTTON_STATE_NONE, //button isn't initialized/doesn't exist
    BUTTON_STATE_DISABLED,
    BUTTON_STATE_ENABLED
};

typedef struct _ProgramState ProgramState;


//NOTE(omar): AFTER CHANGING/ADDING ANYTHING TO THE BUTTON STRUCT
//            MAKE SURE TO EDIT THE Button_init() FUNCTION ACCORDINGLY

typedef struct _Button
{
    int state;
    int x;
    int y;
    int w;
    int h;

    SDL_Color color;
    SDL_Color pressed_color;

    bool mouse_hovering; //true if the mouse is on the button 

    TTF_Font* font;
    char text[256];
    int text_w;
    int text_h;
    bool text_centered; //if false then text will be drawn from 0 on the x coord but cenetered
                        //on the y

    ArrayInt child_buttons;

    void (*on_click)(struct _Button*, ProgramState*);
    void (*on_input)(struct _Button*, ProgramState*, String*);
} Button;


typedef struct
{
    bool disabled;
    int x;
    int y;
    int w;
    int h;

    int r;
    int g;
    int b;
    int pressed_r;
    int pressed_g;
    int pressed_b;

    char* text;
    bool text_centered;
    TTF_Font* font;

    void (*on_click)(Button*, ProgramState*, Button*);
    void (*on_input)(Button*, ProgramState*, String*);
} ButtonConfig;


void Button_init(Button* button, ButtonConfig* config);
void Button_add_child(Button* button, ProgramState* state, int child_index);
void Button_remove_child(Button* button, int child_index /*the index of the child button in the ProgramState array.*/);

void Button_draw(Button* button, SDL_Surface* dest_surface, SDL_Color* bg_color,
                 int offset_x, int offset_y);

void Button_on_mouse_move(Button* button, int mouse_x, int mouse_y, int offset_x, int offset_y);
bool Button_is_mouse_hovering(Button* button);

void Button_resize_text(Button* button, TTF_Font* font);

void Button_disable_children(Button* button, ProgramState* state);

void Button_save_on_click(Button* button, ProgramState* state);
void Button_save_on_input(Button* button, ProgramState* state, String* input);

void Button_open_on_input(Button* button, ProgramState* state, String* input);

void Button_file_on_click(Button* button, ProgramState* state);

//helpers

//TODO(omar): if 2 buttons have the same text. this will return the first one it finds
//Maybe add a "name" member of Button and use that instead for searching
Button* get_button_by_text(const Button* buttons, int button_count, const char* text);
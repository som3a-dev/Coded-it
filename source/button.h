#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>


enum
{
    BUTTON_STATE_DISABLED,
    BUTTON_STATE_ENABLED
};


typedef struct
{
    int state;
    int x;
    int y;
    int w;
    int h;

    SDL_Color color;
    SDL_Color pressed_color;

    bool mouse_hovering; //true if the mouse is on the button 

    const char* text;
    int text_w;
    int text_h;
    bool text_centered; //if false then text will be drawn from 0 on the x coord but cenetered
                        //on the y
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

    const char* text;
    bool text_centered;
    TTF_Font* font;
} ButtonConfig;
void Button_init(Button* button, ButtonConfig* config);
void Button_draw(Button* button, TTF_Font* font, SDL_Surface* dest_surface);

void Button_on_mouse_move(Button* button, int mouse_x, int mouse_y);
void Button_on_mouse_click(Button* button, uint32_t mouse);

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


void Button_init(Button* button, int state, int x, int y, int w, int h,
                uint8_t r, uint8_t g, uint8_t b,
                uint8_t pressed_r, uint8_t pressed_g, uint8_t pressed_b,
                const char* text, bool text_centered,
                TTF_Font* font);
void Button_draw(Button* button, TTF_Font* font, SDL_Surface* dest_surface);

void Button_on_mouse_move(Button* button, int mouse_x, int mouse_y);
void Button_on_mouse_click(Button* button, uint32_t mouse);

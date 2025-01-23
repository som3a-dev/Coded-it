#pragma once
#include <SDL.h>
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
} Button;


void Button_init(Button* button, int state, int x, int y, int w, int h,
                uint8_t r, uint8_t g, uint8_t b,
                uint8_t pressed_r, uint8_t pressed_g, uint8_t pressed_b,
                const char* text);
void Button_draw(Button* button, SDL_Surface* dest_surface);

void Button_on_mouse_move(Button* button, int mouse_x, int mouse_y);
void Button_on_mouse_click(Button* button, uint32_t mouse);

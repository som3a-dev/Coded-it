#include "button.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>


void Button_init(Button* button, int state, int x, int y, int w, int h,
                uint8_t r, uint8_t g, uint8_t b,
                uint8_t pressed_r, uint8_t pressed_g, uint8_t pressed_b,
                const char* text)
{
    button->state = state;
    button->x = x;
    button->y = y;
    button->w = w;
    button->h = h;
    button->color.r = r;
    button->color.g = g;
    button->color.b = b;
    button->pressed_color.r = pressed_r;
    button->pressed_color.g = pressed_g;
    button->pressed_color.b = pressed_b;
    button->text = text;
}


void Button_draw(Button* button, SDL_Surface* dest_surface)
{
    if (button->state == BUTTON_STATE_DISABLED)
    {
        return;
    }
    SDL_Rect rect = {button->x, button->y, button->w, button->h};

    if (button->mouse_hovering == false)
    {
        SDL_FillRect(dest_surface, &rect, SDL_MapRGB(dest_surface->format, button->color.r, button->color.g, button->color.b));
    }
    else
    {
        SDL_FillRect(dest_surface, &rect, SDL_MapRGB(dest_surface->format, button->pressed_color.r, button->pressed_color.g, button->pressed_color.b));
    }
}


void Button_on_mouse_move(Button* button, int mouse_x, int mouse_y)
{
    if (button->state == BUTTON_STATE_DISABLED)
    {
        return;
    }
    
    int right = button->x + button->w;
    int bottom = button->y + button->h;

    bool in_button_x = ((button->x < mouse_x) && (mouse_x < right));
    bool in_button_y = ((button->y < mouse_y) && (mouse_y < bottom));

    if (in_button_x && in_button_y)
    {
        button->mouse_hovering = true;
    }
    else
    {
        button->mouse_hovering = false;
    }
}


void Button_on_mouse_click(Button* button, uint32_t mouse)
{
    if (button->state == BUTTON_STATE_DISABLED)
    {
        return;
    }

    if (mouse & SDL_BUTTON(1))
    {
        printf("Button clicked\n");
    } 
}
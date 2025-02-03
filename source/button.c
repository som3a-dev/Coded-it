#include "button.h"
#include "editor.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <memory.h>

void Button_init(Button* button, ButtonConfig* config)
{
    memset(button, 0, sizeof(Button));
    button->state = BUTTON_STATE_ENABLED;
    if (config->disabled)
    {
        button->state = BUTTON_STATE_DISABLED;
    }
    button->x = config->x;
    button->y = config->y;
    button->w = config->w;
    button->h = config->h;
    button->color.r = config->r;
    button->color.g = config->g;
    button->color.b = config->b;
    button->pressed_color.r = config->pressed_r;
    button->pressed_color.g = config->pressed_g;
    button->pressed_color.b = config->pressed_b;
    button->text = config->text;
    button->text_centered = config->text_centered;
    button->on_click = config->on_click;
    button->on_input = config->on_input;

    if (config->text && config->font)
    {
        TTF_SizeText(config->font, config->text, &(button->text_w), &(button->text_h));
        if (config->w == 0)
        {
            button->w = button->text_w;
        }
        if (config->h == 0)
        {
            button->h = button->text_h;
        }
    }

}


void Button_draw(Button* button, TTF_Font* font, SDL_Surface* dest_surface)
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

    int text_x = button->x;
    int text_y = button->y;

    if (button->text_centered)
    {
        text_x += (button->w / 2) - (button->text_w / 2);
    }
    text_y += (button->h / 2) - (button->text_h / 2);

    draw_text(font, dest_surface, button->text,
              text_x, text_y,
              255, 255, 255);
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


bool Button_is_mouse_hovering(Button* button)
{
    if (button->state == BUTTON_STATE_DISABLED)
    {
        return false;
    }

    return button->mouse_hovering;
}


void Button_save_on_click(ProgramState* state)
{
    if (!state) return;

    editor_set_state(state, EDITOR_STATE_COMMAND_INPUT); 
}


void Button_save_on_input(ProgramState* state, String* input)
{
    if (!state) return;

    if (input)
    {
        if (input->text)
        {
            editor_set_filename(state, input->text); 
            editor_save_file(state);
        }
        else
        {
            printf("Please specify a filename.\n");
        }
    }
}


void Button_open_on_input(ProgramState* state, String* input)
{
    if (!state) return;

    if (input)
    {
        if (input->text)
        {
            editor_set_filename(state, input->text); 
            editor_open_file(state);
        }
        else
        {
            printf("Please specify a filename.\n");
        }
    }
}

#include "button.h"
#include "editor.h"
#include "editor_fileio.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
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
    memcpy(button->text, config->text, sizeof(char) * (strlen(config->text) + 1));
    button->text_centered = config->text_centered;
    button->on_click = config->on_click;
    button->on_input = config->on_input;
    button->font = config->font;

    if (config->text && config->font)
    {
        Button_resize_text(button, config->font);
    }

}

void Button_add_child(Button* button, ProgramState* state, int child_index)
{
    if (!button) return;
    if (button->state == BUTTON_STATE_NONE)
    {
        return;
    }


    Button* child = state->buttons + child_index;
    if (child->state != BUTTON_STATE_NONE)
    {
        ArrayInt_push(&(button->child_buttons), child_index);
    }
}


void Button_remove_child(Button* button, int child_index)
{
    if (!button) return;
    if (button->state == BUTTON_STATE_NONE)
    {
        return;
    }

    ArrayInt_remove(&(button->child_buttons), child_index);
}


void Button_draw(Button* button, SDL_Surface* dest_surface, SDL_Color* bg_color, int offset_x, int offset_y)
{
    if (button->state != BUTTON_STATE_ENABLED)
    {
        return;
    }

    SDL_Rect rect = {button->x, button->y, button->w, button->h};
    rect.x -= offset_x;
    rect.y -= offset_y;

//    printf("%d, %d\n", rect.x, rect.y);

    int text_x = rect.x;
    int text_y = rect.y;


    if (button->text_centered)
    {
        text_x += (button->w / 2) - (button->text_w / 2);
    }
    text_y += (button->h / 2) - (button->text_h / 2);

    if (button->mouse_hovering == false)
    {
        if (bg_color == NULL)
        {
            SDL_FillRect(dest_surface, &rect, SDL_MapRGB(dest_surface->format, button->color.r, button->color.g, button->color.b));
            draw_text(button->font, dest_surface, button->text,
                    text_x, text_y,
                    255, 255, 255,
                    button->color.r, button->color.g, button->color.b);
        }
        else
        {
            draw_text(button->font, dest_surface, button->text,
                    text_x, text_y,
                    255, 255, 255,
                    bg_color->r, bg_color->g, bg_color->b);
        }

    }
    else
    {
        SDL_FillRect(dest_surface, &rect, SDL_MapRGB(dest_surface->format, button->pressed_color.r, button->pressed_color.g, button->pressed_color.b));
        draw_text(button->font, dest_surface, button->text,
                text_x, text_y,
                255, 255, 255,
                button->pressed_color.r, button->pressed_color.g, button->pressed_color.b);
    }
}


bool Button_on_mouse_move(Button* button, int mouse_x, int mouse_y, int offset_x, int offset_y)
{
    if (button->state != BUTTON_STATE_ENABLED)
    {
        return false;
    }

    int x = button->x - offset_x;
    int y = button->y - offset_y; 

    int right = x + button->w;
    int bottom = y + button->h;

    bool in_button_x = ((x < mouse_x) && (mouse_x < right));
    bool in_button_y = ((y < mouse_y) && (mouse_y < bottom));

    if (in_button_x && in_button_y)
    {
        button->mouse_hovering = true;
    }
    else
    {
        button->mouse_hovering = false;
    }

    return button->mouse_hovering;
}


bool Button_is_mouse_hovering(Button* button)
{
    if (button->state != BUTTON_STATE_ENABLED)
    {
        return false;
    }

    return button->mouse_hovering;
}


void Button_resize_text(Button* button, TTF_Font* font)
{
/*    if (button->state != BUTTON_STATE_ENABLED)
    {
        return false;
    }*/

    TTF_SizeText(font, button->text, &(button->text_w), &(button->text_h));
    if (button->w == 0)
    {
        button->w = button->text_w;
    }
    if (button->h == 0)
    {
        button->h = button->text_h;
    }
}


void Button_disable_children(Button* button, ProgramState* state)
{
    if (button->state != BUTTON_STATE_ENABLED)
    {
        return false;
    }

    for (int i = 0; i < button->child_buttons.count; i++)
    {
        int index;
        ArrayInt_get(&(button->child_buttons), i, &index);

        Button* child = state->buttons + index;
        child->state = BUTTON_STATE_DISABLED;
    }
}


void Button_file_name_on_click(Button* button, ProgramState* state)
{
    if (!state) return;
    
    String_set(&(state->current_file), button->text);

    switch (state->file_explorer_action)
    {
        case EXPLORER_ACTION_SAVE:
        {
            editor_save_file(state);
        } break;

        case EXPLORER_ACTION_OPEN:
        {
            editor_open_file(state);
        } break;
    }

    editor_set_state(state, EDITOR_STATE_EDIT);
}


void Button_save_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

//    editor_set_state(state, EDITOR_STATE_COMMAND_INPUT);
    editor_set_state(state, EDITOR_STATE_FILE_EXPLORER);
    state->file_explorer_action = EXPLORER_ACTION_SAVE;
}


void Button_open_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

//    editor_set_state(state, EDITOR_STATE_COMMAND_INPUT);
    editor_set_state(state, EDITOR_STATE_FILE_EXPLORER);
    state->file_explorer_action = EXPLORER_ACTION_OPEN;
}


void Button_save_on_input(Button* button, ProgramState* state, String* input)
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


void Button_open_on_input(Button* button, ProgramState* state, String* input)
{
    if (!state) return;
    return;

    if (input)
    {
        if (input->text)
        {
//            editor_set_filename(state, input->text); 
//            editor_open_file(state);
        }
        else
        {
            printf("Please specify a filename.\n");
        }
    }
}


void Button_file_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

    if (button->child_buttons.elements)
    {
        for (int i = 0; i < button->child_buttons.count; i++)
        {
            int index;
            ArrayInt_get(&(button->child_buttons), i, &index);

            Button* child = state->buttons + index;

            if (child->state == BUTTON_STATE_ENABLED)
            {
                child->state = BUTTON_STATE_DISABLED;
            }
            else
            {
                child->state = BUTTON_STATE_ENABLED;
            }
        }
    }
}


Button* get_button_by_text(const Button* buttons, int button_count, const char* text)
{
    for (int i = 0; i < button_count; i++)
    {
        if (buttons[i].state != BUTTON_STATE_NONE)
        {
            if (strcmp(text, buttons[i].text) == 0)
            {
                return buttons + i;
            }
        }
    }
    
    return NULL;
}
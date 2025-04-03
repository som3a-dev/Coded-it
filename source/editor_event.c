#include "editor_event.h"


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))



void editor_handle_events(ProgramState* state)
{
    SDL_Event e;
    
    SDL_WaitEventTimeout(&e, 1000);
    
    switch (e.type)
    {
        case SDL_QUIT:
        {
            state->running = false;
        } break;
        
        case SDL_TEXTINPUT:
        {
            InputBuffer* buffer = editor_get_current_input_buffer(state);
            if (buffer)
            {
                String_insert(&(buffer->text), e.text.text[0], buffer->cursor_index);
                //system("@cls||clear");
                //printf("%s\n", state->text);
                
                editor_set_cursor(state, buffer->cursor_index + 1);

                TextAction action = {0};
                action.type = TEXT_ACTION_WRITE; 
                action.character_index = buffer->cursor_index - 1;
                action.character = e.text.text[0];

                editor_push_text_action(state, &action);
            }
        } break;

        case SDL_MOUSEMOTION:
        {
            for (int i = 0; i < 10; i++)
            {
                Button_on_mouse_move(state->buttons + i, e.motion.x, e.motion.y);
            }
        } break;

        case SDL_KEYUP:
        {
            if (state->state == EDITOR_STATE_EDIT)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_LSHIFT:
                    {
/*                        InputBuffer* buffer = editor_get_current_input_buffer(state);
                        if (state->selection_start_index != -2)
                        {
                            int selection_start = MIN(state->selection_start_index, buffer->cursor_index);
                            int selection_end = MAX(state->selection_start_index, buffer->cursor_index);

                            for (int i = selection_start; i <= selection_end; i++)
                            {
                                printf("%c", buffer->text.text[i]);
                            }
                            printf("\n");
                        }*/
                    } break;
                }
            }
        } break;
        
        case SDL_KEYDOWN:
        {
            editor_handle_events_keydown(state, e);
        } break;
    }
}


void editor_handle_events_keydown(ProgramState* state, SDL_Event e)
{
    if (state->state == EDITOR_STATE_COMMAND)
    {
        editor_handle_events_keydown_command(state, e);
    }
    else
    {
        editor_handle_events_keydown_textual(state, e);
    }

    switch (e.key.keysym.sym)
    {
        case SDLK_o:
        {
            uint8_t* keystate = SDL_GetKeyboardState(NULL);

            if (SDL_is_ctrl_pressed(keystate))
            {
                int new_state = state->state + 1;

                if (new_state >= EDITOR_STATE_COUNT)
                {
                    new_state = EDITOR_STATE_EDIT; //TODO(omar): maybe we should set to zero instead.
                }

                editor_set_state(state, new_state);
            }
        } break;

        case SDLK_F11:
        {
            uint32_t flags = SDL_GetWindowFlags(state->window);
            if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
            {
                SDL_SetWindowFullscreen(state->window, 0);
                state->window_surface = SDL_GetWindowSurface(state->window);
                SDL_GetWindowSize(state->window, &(state->window_w), &(state->window_h));
            }
            else
            {
                SDL_SetWindowFullscreen(state->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                state->window_surface = SDL_GetWindowSurface(state->window);
                SDL_GetWindowSize(state->window, &(state->window_w), &(state->window_h));
            }

            editor_resize_and_position_buttons(state);
        } break;

        case SDLK_EQUALS:
        {
            uint8_t* keystate = SDL_GetKeyboardState(NULL);

            if (SDL_is_ctrl_pressed(keystate))
            {
                TTF_CloseFont(state->font);

                state->font_size += 2;
                if (state->font_size > 36)
                {
                    state->font_size = 36;
                }

                state->font = TTF_OpenFont("CONSOLA.ttf", state->font_size);
                TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));
                
                editor_resize_and_position_buttons(state);
            }
        } break;

        case SDLK_MINUS:
        {
            uint8_t* keystate = SDL_GetKeyboardState(NULL);

            if (SDL_is_ctrl_pressed(keystate))
            {
                TTF_CloseFont(state->font);

                state->font_size -= 2;
                if (state->font_size < 12)
                {
                    state->font_size = 12;
                }

                state->font = TTF_OpenFont("CONSOLA.ttf", state->font_size);
                TTF_SizeText(state->font, "A", &(state->char_w), &(state->char_h));

                editor_resize_and_position_buttons(state);
            }
        } break;
    }
}


void editor_handle_events_keydown_command(ProgramState* state, SDL_Event e)
{
    switch (e.key.keysym.sym)
    {
        case SDLK_UP:
        {
            if (state->clicked_button == NULL)
            {
                editor_select_first_enabled_button(state);
            }
            else
            {
                for (int i = 1; i < 10; i++)
                {
                    Button* button = state->buttons + i;
                    if (button == state->clicked_button &&
                        (button->state == BUTTON_STATE_ENABLED))
                    {
                        if ((state->buttons + i - 1)->state != BUTTON_STATE_ENABLED)
                        {
                            continue;
                        }

                        state->clicked_button->mouse_hovering = false;
                        state->clicked_button = state->buttons + i - 1;
                        break;
                    }
                }
            }
            state->clicked_button->mouse_hovering = true;
        } break;

        case SDLK_DOWN:
        {
            if (state->clicked_button == NULL)
            {
                editor_select_first_enabled_button(state);
            }
            else
            {
                for (int i = 0; i < 9; i++)
                {
                    Button* button = state->buttons + i;
                    if (button == state->clicked_button &&
                        (button->state == BUTTON_STATE_ENABLED))
                    {
                        if ((state->buttons + i + 1)->state != BUTTON_STATE_ENABLED)
                        {
                            continue;
                        }

                        state->clicked_button->mouse_hovering = false;
                        state->clicked_button = state->buttons + i + 1;
                        break;
                    }
                }
            }

            state->clicked_button->mouse_hovering = true;
        } break;

        case SDLK_RETURN:
        {
            if (state->clicked_button)
            {
                if (state->clicked_button->on_click)
                {
                    state->clicked_button->on_click(state->clicked_button, state);
                }
            }
        } break;
    }
}


void editor_handle_events_keydown_textual(ProgramState* state, SDL_Event e)
{
    InputBuffer* buffer = editor_get_current_input_buffer(state);
    uint8_t* keystate = SDL_GetKeyboardState(NULL);

    bool abort_selection = true;

    switch (e.key.keysym.sym)
    {
        case SDLK_LSHIFT:
        {
            abort_selection = false;

            if (state->selection_start_index == -2)
            {
                state->selection_start_index = buffer->cursor_index;
            }
        } break;
        
        case SDLK_z: //undo
        {
            if (SDL_is_ctrl_pressed(keystate))
            {
                TextAction* last_action = Stack_pop(&(state->undo_tree), true);

                if (last_action)
                {
                    printf("Last action\n{\n    Type: %d\n    Character: %c\n    Index: %d\n}\n",
                    last_action->type, last_action->character, last_action->character_index);

                    editor_undo_text_action(state, last_action);
                }
            }
        } break;

        case SDLK_r: //redo
        {
            if (SDL_is_ctrl_pressed(keystate))
            {
                TextAction* last_action = Stack_pop(&(state->redo_tree), true);
                
                if (last_action)
                {
                    printf("Last action\n{\n    Type: %d\n    Character: %c\n    Index: %d\n}\n",
                    last_action->type, last_action->character, last_action->character_index);

                    editor_redo_text_action(state, last_action);
                }
            }
        } break;

        case SDLK_c:
        {
            if (state->selection_start_index != -2)
            {
                if (SDL_is_ctrl_pressed(keystate))
                {
                    int selection_start = -2;
                    int selection_end = -2;
                    if (state->selection_start_index < (buffer->cursor_index))
                    {
                        selection_start = MIN(state->selection_start_index, buffer->cursor_index);
                        selection_end = MAX(state->selection_start_index, buffer->cursor_index);
                    }
                    else
                    {
                        selection_start = MIN(state->selection_start_index, buffer->cursor_index);
                        selection_end = MAX(state->selection_start_index, buffer->cursor_index);
                    }

                    int len = (selection_end - selection_start) + 1;
                    char* text = buffer->text.text + selection_start;
                    char* text_copy = malloc(sizeof(char) * (len+1));

                    memcpy(text_copy, text, sizeof(char) * len);

                    text_copy[len] = '\0';
                    
                    String_set(&(state->clipboard), text_copy);

                    free(text_copy);

                    printf("%s\n", state->clipboard.text);
                }
            }
        } break;

        case SDLK_v:
        {
            if (state->clipboard.text)
            {
                if (SDL_is_ctrl_pressed(keystate))
                {
                    InputBuffer* buffer = editor_get_current_input_buffer(state);
                    printf("%s\n", state->clipboard.text);
                    String_insert_string(&(buffer->text), state->clipboard.text,
                    buffer->cursor_index);
                    
                    buffer->cursor_index += state->clipboard.len;
                }
            }
        } break;

        case SDLK_BACKSPACE:
        {
            //String_pop(&(state->text));

            if (state->selection_start_index == -2)
            {
                String_remove(&(buffer->text), buffer->cursor_index - 1);
                editor_set_cursor(state, buffer->cursor_index - 1);
            }
            else
            {
                int selection_start, selection_end;
                if (state->selection_start_index < (buffer->cursor_index))
                {
                    selection_start = MIN(state->selection_start_index, buffer->cursor_index);
                    selection_end = MAX(state->selection_start_index, buffer->cursor_index);
                }
                else
                {
                    selection_start = MIN(state->selection_start_index, buffer->cursor_index);
                    selection_end = MAX(state->selection_start_index, buffer->cursor_index);
                }

                for (int i = selection_start; i <= selection_end; i++)
                {
                    String_remove(&(state->text.text), selection_start);
                }

                editor_set_cursor(state, selection_end - (selection_end - selection_start));
            }
        } break;
        
        case SDLK_RETURN:
        {
            //String_push(&(state->text), '\n');
            switch (state->state)
            {
                case EDITOR_STATE_EDIT:
                {
                    String_insert(&(buffer->text), '\n', buffer->cursor_index);
                    editor_set_cursor(state, buffer->cursor_index + 1);
                } break;

                case EDITOR_STATE_COMMAND_INPUT:
                {
                    editor_set_state(state, EDITOR_STATE_EDIT);
                } break;
            }
        } break;
        
        case SDLK_UP:
        {
            if (keystate[SDL_SCANCODE_LSHIFT] != 0)
            {
                abort_selection = false;
            }

            int prev_newline = String_get_previous_newline(&(buffer->text),
            buffer->cursor_index);
            if (prev_newline == -1)
            {
                return;
            }
            
            int cursor_index_in_line = buffer->cursor_index - prev_newline - 1;
            printf("Cursor index in line: %d\n", cursor_index_in_line);
            
            int newline_before_prev_newline = String_get_previous_newline(buffer,
            prev_newline);
            
            //cap cursor_index_in_line at the length of the previous line - 1
            int prev_line_len = prev_newline - newline_before_prev_newline;
            if (cursor_index_in_line >= prev_line_len)
            {
                cursor_index_in_line = prev_line_len - 1;
            }
            
            editor_set_cursor(state,
            newline_before_prev_newline + cursor_index_in_line + 1);
        } break;
        
        case SDLK_DOWN:
        {
            if (keystate[SDL_SCANCODE_LSHIFT] != 0)
            {
                abort_selection = false;
            }

            int prev_newline = String_get_previous_newline(&(buffer->text),
            buffer->cursor_index);
            
            int cursor_index_in_line = buffer->cursor_index - prev_newline - 1;
            //printf("Cursor index in line: %d\n", cursor_index_in_line);
            
            int next_newline = String_get_next_newline(&(buffer->text),
            prev_newline);
            //printf("Next newline: %d\n", next_newline);
            
            if (next_newline == buffer->text.len)
            {
                //We are at the last line
                break;
            }
            
            int next_next_newline = String_get_next_newline(&(buffer->text),
            next_newline);
            //printf("Next newline: %d\n", next_next_newline);
            
            //cap cursor_index_in_line at the length of the previous line - 1
            int next_line_len = next_next_newline - next_newline - 1;
            if (next_line_len < 0) next_line_len = 0;
            //printf("Next line len: %d\n", next_line_len);
            
            if (cursor_index_in_line > next_line_len)
            {
                cursor_index_in_line = next_line_len;
            }
            
            editor_set_cursor(state, next_newline + cursor_index_in_line + 1);
        } break;
        
        case SDLK_LEFT:
        {
            if (keystate[SDL_SCANCODE_LSHIFT] != 0)
            {
                abort_selection = false;
            }

            editor_set_cursor(state, buffer->cursor_index - 1);
        } break;
        
        case SDLK_RIGHT:
        {
            if (keystate[SDL_SCANCODE_LSHIFT] != 0)
            {
                abort_selection = false;
            }

            editor_set_cursor(state, buffer->cursor_index + 1);
        } break;
        
        case SDLK_TAB:
        {
            String_insert(&(buffer->text), ' ', buffer->cursor_index);
            editor_set_cursor(state, buffer->cursor_index+1);
            String_insert(&(buffer->text), ' ', buffer->cursor_index);
            editor_set_cursor(state, buffer->cursor_index+1);
        } break;

        case SDLK_INSERT:
        {
            editor_save_file(state);
        } break;

        case SDLK_LCTRL:
        case SDLK_RCTRL:
        {
            abort_selection = false;
        } break;
    }

    if (abort_selection)
    {
        state->selection_start_index = -2;
    }
}


#include <stdio.h>

#include "button_callback.h"
#include "editor_fileio.h"

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
            editor_save_file(state, input->text);
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

void Button_file_name_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

    int code = 0;
    switch (state->file_explorer_action)
    {
        case EXPLORER_ACTION_SAVE:
        {
            code = editor_save_file(state, button->text);
        } break;

        case EXPLORER_ACTION_OPEN:
        {
            code = editor_open_file(state, button->text);
        } break;
    }

    switch (code)
    {
        case FILEIO_PATH_WAS_DIRECTORY:
        {
            editor_update_file_explorer(state);
        } break;

        case FILEIO_PATH_WAS_FILE:
        {
            editor_set_state(state, EDITOR_STATE_EDIT);
        } break;

        default:
        {
            //TODO(omar): error message
        }
    }
}

#include <stdio.h>
#include <windows.h>

#include "button_callback.h"
#include "editor_fileio.h"
#include "theme_parser.h"

void Button_save_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

    editor_set_state(state, EDITOR_STATE_FILE_EXPLORER);
    state->file_explorer_action = EXPLORER_ACTION_SAVE;
}


void Button_open_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

    editor_set_state(state, EDITOR_STATE_FILE_EXPLORER);
    state->file_explorer_action = EXPLORER_ACTION_OPEN;
}


void Button_load_theme_on_click(Button* button, ProgramState* state)
{
    if (!state) return;

//    editor_set_state(state, EDITOR_STATE_COMMAND_INPUT);
    editor_set_state(state, EDITOR_STATE_FILE_EXPLORER);
    state->file_explorer_action = EXPLORER_ACTION_LOAD_THEME;
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

        case EXPLORER_ACTION_LOAD_THEME:
        {
            String filepath = {0};

            String_insert_string(&filepath, button->text, 0);
            String_insert(&filepath, '\\', 0);
            String_insert_string(&filepath, state->current_directory_ib.text.text, 0);
 
            if (tp_load_theme(state, filepath.text) == false)
            {
                int attributes = GetFileAttributes(filepath.text);
                if (attributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    String_set(&(state->current_directory_ib.text), filepath.text);
                    code = FILEIO_PATH_WAS_DIRECTORY;
                }
            }

            String_clear(&filepath);

            code = FILEIO_PATH_WAS_FILE;
        } break;
    }

    switch (code)
    {
        case FILEIO_PATH_WAS_DIRECTORY:
        {
            editor_update_file_explorer(state);
            state->current_directory_ib.cursor_index = state->current_directory_ib.text.len;
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

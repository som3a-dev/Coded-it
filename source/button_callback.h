#pragma once

#include "button.h"
#include "editor.h"

//The file name in the file explorer listing
void Button_file_name_on_click(Button* button, ProgramState* state);

void Button_save_on_click(Button* button, ProgramState* state);
void Button_save_on_input(Button* button, ProgramState* state, String* input);

void Button_open_on_click(Button* button, ProgramState* state);
void Button_open_on_input(Button* button, ProgramState* state, String* input);

void Button_load_theme_on_click(Button* button, ProgramState* state);

void Button_file_on_click(Button* button, ProgramState* state);



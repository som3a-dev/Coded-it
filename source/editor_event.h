#pragma once

#include "editor.h"
#include <stdbool.h>


//returns true if the editor should update and render
void editor_handle_events(ProgramState* state, bool* should_update);

void editor_handle_events_keydown(ProgramState* state, SDL_Event e);
void editor_handle_events_keydown_command(ProgramState* state, SDL_Event e);
void editor_handle_events_keydown_textual(ProgramState* state, SDL_Event e);
void editor_handle_events_keydown_file_explorer(ProgramState* state, SDL_Event e);

bool editor_navigate_buttons_with_keys(ProgramState* state, Button* button, int button_count, SDL_Event e);

#pragma once
#include "editor.h"


void editor_draw_input_buffer(ProgramState* state);

void editor_draw_cursor(ProgramState* state, const TTF_Font* font);

InputBuffer* editor_get_current_input_buffer(const ProgramState* state);
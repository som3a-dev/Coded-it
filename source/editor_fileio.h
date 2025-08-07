#pragma once

#include "editor.h"




void editor_save_file(const ProgramState* state, const char* filename);

void editor_open_file(ProgramState* state, const char* filename);

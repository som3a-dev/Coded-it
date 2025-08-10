#pragma once

#include "editor.h"


enum
{
    FILEIO_PATH_WAS_INVALID,
    FILEIO_PATH_WAS_DIRECTORY,
    FILEIO_PATH_WAS_FILE
};

//Both return from the FILEIO enum
int editor_save_file(const ProgramState* state, const char* filename);
int editor_open_file(ProgramState* state, const char* filename);

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "string.h"
#include "queue.h"
#include "editor.h"


int main(int argc, char** argv)
{
    ProgramState state;
    editor_init(&state);
    
    editor_loop(&state);
    
    editor_destroy(&state);

    return EXIT_SUCCESS;
}
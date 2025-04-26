#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "string.h"
#include "syntax_highlight.h"
#include "array.h"
#include "queue.h"
#include "editor.h"


int main(int argc, char** argv)
{
    const char* code = "\n\nint main(void)\n{\n     return 0;\n}\n\n";
    printf("%s", code);
    parse(code);


/*   ProgramState state;
    editor_init(&state);
    
    editor_loop(&state);
    
    editor_destroy(&state);*/

    return EXIT_SUCCESS;
}
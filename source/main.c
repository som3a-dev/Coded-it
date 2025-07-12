#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "json_parser.h"
#include "hash_table.h"
#include "string.h"
#include "syntax_parser.h"
#include "array.h"
#include "queue.h"
#include "editor.h"


int main(int argc, char** argv)
{
    ProgramState state;
    editor_init(&state);
    
    editor_loop(&state);
    
    editor_destroy(&state);

    json_object* obj = jp_parse_file("test.json");
    json_value val = {JSON_VALUE_OBJECT, obj};

    json_value_destroy(&val);

    return EXIT_SUCCESS;
}
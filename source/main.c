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
/*    editor_init(&state);
    
    editor_loop(&state);
    
    editor_destroy(&state);*/

    json_object* obj = jp_parse_file("test.json");
    json_value val = {JSON_VALUE_OBJECT, obj};
    json_value_print(&val);

    // hash_table table;
    // hash_table_init(&table, 0, sizeof(String));

    // String str = {0};
    // String_set(&str, "omar");

    // hash_table_set(&table, "name", &str);

    // str.text = NULL;
    // String_set(&str, "mora");

    // hash_table_set(&table, "type", &str);


    // {
    //     String* str2 = hash_table_get(&table, "type");
    //     printf("%s\n", str2->text);
    // }

    return EXIT_SUCCESS;
}
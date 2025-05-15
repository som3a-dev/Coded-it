#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "json_parser.h"
#include "string.h"
#include "hash_table.h"




enum
{
    JP_TOKEN_NONE,
    JP_TOKEN_KEY,
    JP_TOKEN_VALUE
};


bool is_index_part_of_literal(const char* text, int index);


void jp_parse_file(const char* path)
{
    FILE* fp;
    fopen_s(&fp, path, "r");

    if (!fp)
    {
        printf("JSON PARSER: invalid file path '%s'\n", path);
        return;
    }

    hash_table table;
    hash_table_init(&table, 0, sizeof(String));

    char c;
    String token = {0};
    int token_type = JP_TOKEN_NONE;

    String current_key = {0};
    while (!feof(fp))
    {
        c = fgetc(fp);

        switch (c)
        {
            case '"':
            {
                if (token_type == JP_TOKEN_NONE)
                {
                    token_type = JP_TOKEN_KEY;
                    continue;
                }
                else if (token_type == JP_TOKEN_KEY)
                {
                    token_type = JP_TOKEN_NONE;
                    printf("Key: %s\n", token.text);

                    String_set(&current_key, token.text);
                    String_clear(&token);
                }
            } break;

            case ':':
            {
                if (token_type == JP_TOKEN_NONE)
                {
                    token_type = JP_TOKEN_VALUE;
                    continue;
                }
            } break;

            case '{':
            {
                if (token_type == JP_TOKEN_VALUE)
                {
                    if (is_index_part_of_literal(token.text, token.len) == false)
                    {
                        //the '{' is not part of a string literal. treat it accordingly
                //        token_type = JP_TOKEN_NONE;
                    }
                }
            } break;

            case '\n':
            {
                if (token_type == JP_TOKEN_VALUE)
                {
                    token_type = JP_TOKEN_NONE;

                    if (token.text[token.len-1] == ',')
                    {
                        String_pop(&token);
                    }
                    printf("Value: %s\n", token.text);

                    if (current_key.text)
                    {
                        hash_table_set(&table, current_key.text, &token);
                    }

                    String_clear_without_freeing(&token);
                }
            } break;
        }

        if (token_type && (c != ' '))
        {
            String_push(&token, c);
        }
    }


    hash_table_print(&table);
    printf("\n");
    for (int i = 0; i < table.len; i++)
    {
        String* str = hash_table_get_by_index(&table, i);
        if (str)
        {
            if (str->text)
            {
                printf("%d: %s\n", i, str->text);
                String_clear(str);
            }
        }
    }

    String_clear(&current_key);
    String_clear(&token);

    hash_table_clear(&table);
    fclose(fp);
}


bool is_index_part_of_literal(const char* text, int index)
{
    int quote_count = 0;
    for (int i = 0; i < index; i++)
    {
        if (text[i] == '"')
        {
            quote_count++;
        }
    }

    return ((quote_count % 2) != 0);
}










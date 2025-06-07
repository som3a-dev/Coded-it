#include <malloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "json_parser.h"
#include "string.h"
#include "hash_table.h"


enum
{
    JSON_TOKEN_NONE,
    JSON_TOKEN_STRING,
    JSON_TOKEN_INT,
    JSON_TOKEN_BOOL,
    JSON_TOKEN_CHAR
};

typedef struct
{
    int type;

    //this is a real allocated pointer only in the case of strings
    //in other cases (int, char) this is an encoded pointer
    //meaning the pointer variable directly stores the value of the token
    //no memory allocation needed
    char* val;
} json_token;


static inline void json_token_set_int(json_token* token, int val);
static inline void json_token_set_string(json_token* token, char* val);
static inline void json_token_set_bool(json_token* token, bool val);
static inline void json_token_set_char(json_token* token, char val);

void json_token_print(json_token* token);

void json_token_destroy(json_token* token);


//helper for jp_lex()
static void _resize_tokens_array(json_token** tokens, int* tokens_count, int new_count);


bool is_index_part_of_literal(const char* text, int index);

void jp_lex(const char* str);

String jp_lex_string(const char* str);

String jp_lex_numeric(const char* str);

//str isn't changed after the function is done. it is mutated in the function
//and then reset back to its original state
//so it is practically const
String jp_lex_bool(char* str);


void jp_parse_file(const char* path)
{
    FILE* fp;
    fopen_s(&fp, path, "rb");

    if (!fp)
    {
        printf("JSON PARSER: invalid file path '%s'\n", path);
        return;
    }

    char* file_string = NULL;
    long file_len = 0;

    fseek(fp, 0L, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    file_string = malloc(sizeof(char) * (file_len+1));
    fread_s(file_string, sizeof(char) * file_len, sizeof(char), file_len, fp);
    file_string[file_len] = '\0';

    printf("%s\n", file_string);

    //lexing
    jp_lex(file_string);

    fclose(fp);
    free(file_string);
}


void jp_lex(const char* str)
{
    json_token* tokens = NULL;
    int tokens_count = 0;

    while (str[0])
    {
        String json_str = jp_lex_string(str);
        if (json_str.text)
        {
            str += json_str.len;

            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            //DON'T CLEAR THE json_str.
            //json_token_set_string doesn't copy the string. it just uses the existing one in json_str
            //the string should be freed later
            json_token_set_string(tokens + (tokens_count-1), json_str.text);

//            printf("%s\n", tokens[tokens_count-1].val);
            continue;
        }

        String json_numeric = jp_lex_numeric(str);
        if (json_numeric.text)
        {
            str += json_numeric.len;

            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            int json_numeric_value = atoi(json_numeric.text);
            json_token_set_int(tokens + (tokens_count-1), json_numeric_value);

            String_clear(&json_numeric);
            continue;
        }

        String json_bool = jp_lex_bool(str);
        if (json_bool.text)
        {
//            printf("%s\n", json_bool.text);
            str += json_bool.len;

            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            bool json_bool_value = false;
            if (strcmp(json_bool.text, "true") == 0)
            {
                json_bool_value = true;
            }
            json_token_set_bool(tokens + (tokens_count-1), json_bool_value);

            String_clear(&json_bool);
            continue;
        }

        switch (str[0])
        {
            case ':':
            case '[':
            case ']':
            case '{':
            case '}':
            case ',':
            {
//                printf("%c\n", str[0]);
                _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);
                json_token_set_char(tokens + (tokens_count-1), str[0]);
            } break;
        }
        
        str++;
    }

    for (int i = 0; i < tokens_count; i++)
    {
        json_token_print(tokens + i);
    }

    for (int i = 0; i < tokens_count; i++)
    {
        json_token_destroy(tokens + i);
    }
}


String jp_lex_string(const char* str)
{
    String result = {0};
    int len = strlen(str);

    if (str[0] == '"')
    {
        String_push(&result, '"');
        for (int i = 1; i < len; i++)
        {
            if (str[i] == '"')
            {
                break;
            }

            String_push(&result, str[i]);
        }
        String_push(&result, '"');
    }

    return result;
}


String jp_lex_numeric(const char* str)
{
    String number_string = {0}; //the string containing the number we lexed
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        if (str[i] < '0') break;
        if (str[i] > '9') break;

        String_push(&number_string, str[i]);
    }

    return number_string;
}


String jp_lex_bool(char* str)
{
    String bool_string = {0};
    int len =  strlen(str);

    //check for true
    if (len >= 4)
    {
        char temp = str[4];
        str[4] = '\0';

        if (strcmp(str, "true") == 0)
        {
            String_set(&bool_string, "true");
            str[4] = temp;
            return bool_string;
        }

        str[4] = temp;
    }

    //check for false
    if (len >= 5)
    {
        char temp = str[5];
        str[5] = '\0';

        if (strcmp(str, "false") == 0)
        {
            String_set(&bool_string, "false");
            str[5] = temp;
            return bool_string;
        }

        str[5] = temp;
    }

    return bool_string;
}


static void _resize_tokens_array(json_token** tokens, int* tokens_count, int new_count)
{
    if (new_count < 0) return;
    if (new_count == *tokens_count) return;

    if ((*tokens))
    {
        (*tokens) = realloc(*tokens, new_count * sizeof(json_token));

        if (new_count > (*tokens_count))
        {
            json_token* new_memory = (*tokens) + (*tokens_count);
            memset(new_memory, 0, sizeof(json_token) * (new_count - (*tokens_count)));
        }
    }
    else
    {
        (*tokens) = calloc(new_count, sizeof(json_token));
    }

    *tokens_count = new_count;
}


static inline void json_token_set_int(json_token* token, int val)
{
    token->type = JSON_TOKEN_INT;
    token->val = val;
}


static inline void json_token_set_string(json_token* token, char* val)
{
    token->type = JSON_TOKEN_STRING;
    token->val = val;
}


static inline void json_token_set_bool(json_token* token, bool val)
{
    token->type = JSON_TOKEN_BOOL;
    token->val = val;
}


static inline void json_token_set_char(json_token* token, char val)
{
    token->type = JSON_TOKEN_CHAR;
    token->val = (char *)(uintptr_t)(unsigned char)val;
}


void json_token_print(json_token* token)
{
    switch (token->type)
    {
        case JSON_TOKEN_STRING:
        {
            printf("%s\n", token->val);
        } break;

        case JSON_TOKEN_INT:
        {
            printf("%d\n", token->val);
        } break;

        case JSON_TOKEN_BOOL:
        {
            const char* str = "true";
            if (token->val == 0)
            {
                str = "false";
            }
            printf("%s\n", str);
        } break;

        case JSON_TOKEN_CHAR:
        {
            printf("%c\n", token->val);
        } break;
    }
}


void json_token_destroy(json_token* token)
{
    if (token->type == JSON_TOKEN_STRING)
    {
        if (token->val)
        {
            free(token->val);
            token->val = NULL;
        }
    }
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











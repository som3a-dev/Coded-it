//sp_ prefix stands for syntax parser

#pragma once

#include <stdbool.h>

enum
{
    TOKEN_NONE,
    TOKEN_KEYWORD
};

typedef struct
{
    int start_index;
    int end_index;
    int type;
    String text;
} Token;


bool sp_is_keyword(const char* text);

int sp_get_token_type(const char* token);
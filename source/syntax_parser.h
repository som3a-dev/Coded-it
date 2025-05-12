//sp_ prefix stands for syntax parser

#pragma once

#include <stdbool.h>

enum
{
    TOKEN_NONE,
    TOKEN_KEYWORD,
    TOKEN_NUMERIC,
    TOKEN_STRING_LITERAL,
    TOKEN_BRACES,
    _TOKEN_COUNT
};

typedef struct
{
    int start_index;
    int end_index;
    int type;
    String text;
} Token;


bool sp_is_keyword(const char* text);
bool sp_is_numeric(const char* text);
bool sp_is_string_literal(const char* text);

int sp_get_token_type(const char* token);
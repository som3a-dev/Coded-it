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
    TOKEN_COMMENT,
    _TOKEN_COUNT
};

//This struct is passed to the syntax parser from the editor text rendering
//code. It contains metadata about the file both constant and variable
//and the current token
//used in parsing the token and finding its type
typedef struct
{
    int quote_count; //how many quotes in the file up to this point
    bool line_is_comment;
} sp_metadata;


bool sp_is_keyword(const char* text);
bool sp_is_numeric(const char* text);

//NOTE(omar): IMPORTANT NOTES ABOUT HOW THIS FUNCTION AND STRING LITERALS WORK
//This function only returns true IF the token is JUST A string. as in the first and last character in it are 
//similar quotations. it does NOT return true if the token fails that condition but has a string INSIDE it.
//THAT case is handled in the text rendering code itself.
//So the only thing useful about this function is that it saves us some time if the token is clearly simply a string
//we just render it without checking for strings each character. thats it.
//This is not very clean. But given that we aren't willing to sacrifice our 'token' model to call sp_get_token_type
//for every single character. this is what we currently do. Tough Luck!
//NOTE(omar): this function always returns false when an sp_metadata md is passed. meaning it does nothing as of now and is just for consistency
//TODO(omar): Rewrite all this shit and decouple the text rendering code from highlighting string literals or comments or anything
bool sp_is_string_literal(const char* text, sp_metadata* md);

bool sp_is_braces(const char* text);

//NOTE(omar): EXACT SAME NOTE AS THE STRING LITERALS
//THE EXACT SAME LOGIC
bool sp_is_comment(const char* text, sp_metadata* md);

bool sp_is_delimiter(char c);

int sp_get_token_type(const char* token, sp_metadata* md);

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "string.h"
#include "syntax_parser.h"

const char* keywords = "auto:break:case:char:const:continue:default:do:double:else:enum:extern:float:for:goto:if:inline:int:long:register:restrict:return:short:signed:sizeof:static:struct:switch:typedef:union:unsigned:void:volatile:while:_Bool:_Complex:_Imaginary";


int sp_get_token_type(const char* token, sp_metadata* md)
{
    if (sp_is_comment(token, md))
    {
        return TOKEN_COMMENT;
    }
    if (sp_is_keyword(token))
    {
        return TOKEN_KEYWORD;
    }
    if (sp_is_numeric(token))
    {
        return TOKEN_NUMERIC;
    }
    if (sp_is_string_literal(token, md))
    {
        return TOKEN_STRING_LITERAL;
    }

    if (strlen(token) == 1)
    {
        switch (token[0])
        {
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            {
                return TOKEN_BRACES;
            } break;
        }
    }

    return TOKEN_NONE;
}


bool sp_is_comment(const char* text, sp_metadata* md)
{
    if (md)
    {
        if (md->line_is_comment)
        {
            return true;
        }
    }

    if (strlen(text) < 2)
    {
        return false;
    }

    if ((text[0] == text[1]) && (text[1] == '/'))
    {
        return true;
    }

    return false;
}


bool sp_is_braces(const char* text)
{
    if (strlen(text) == 1)
    {
        switch (text[0])
        {
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            {
                return true;
            } break;
        }
    }

    return false;
}


bool sp_is_string_literal(const char* text, sp_metadata* md)
{
    if (md)
    {
        return false;
    }

    int len = strlen(text);

    if ((text[0] == '\'') && (text[len-1] == '\''))
    {
        return true;
    }

    if ((text[0] == '"') && (text[len-1] == '"'))
    {
        return true;
    }

    return false;
}


bool sp_is_numeric(const char* text)
{
    char c;
    for (int i = 0; i < strlen(text); i++)
    {
        c = text[i];

        if ((c != '.')) 
        {
            if ((c < '0') || (c > '9'))
            {
                if (i != (strlen(text)-1))
                {
                    return false;
                }
                
                //this is the last character. being an f or F (float) is valid
                if ((c == 'F') || (c == 'f'))
                {
                    return true;
                }

                return false;
            }
        }
    }

    return true;
}


bool sp_is_keyword(const char* text)
{
    int keywords_len = strlen(keywords); 
    char* keywords_copy = calloc(keywords_len+1, sizeof(char));
    memcpy(keywords_copy, keywords, sizeof(char) * keywords_len);

    char* keyword_start = keywords_copy;
    for (int j = 0; j <= keywords_len; j++)
    {
        if (keywords_copy[j] == ':' || keywords_copy[j] == '\0')
        {
            keywords_copy[j] = '\0';

            if (strcmp(text, keyword_start) == 0)
            {
                return true;
            }

            if (j != keywords_len)
            {
                keywords_copy[j] = ':';
                keyword_start = keywords_copy + j + 1;
            }
        }
    }
    free(keywords_copy);

    return false;
}


bool sp_is_delimiter(char c)
{
    const char* delimiters = " \n(){}[];*+-|&%#";
    const int delimiters_len = strlen(delimiters);

    for (int i = 0; i < delimiters_len; i++)
    {
        if (c == delimiters[i])
        {
            return true;
        }
    }

    return false;
}

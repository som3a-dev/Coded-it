#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "string.h"
#include "syntax_parser.h"

const char* keywords = "auto:break:case:char:const:continue:default:do:double:else:enum:extern:float:for:goto:if:inline:int:long:register:restrict:return:short:signed:sizeof:static:struct:switch:typedef:union:unsigned:void:volatile:while:_Bool:_Complex:_Imaginary";





int sp_get_token_type(const char* token)
{
    if (sp_is_keyword(token))
    {
        return TOKEN_KEYWORD;
    }

    return TOKEN_NONE;
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


Token* sp_parse(const char* text, int* out_tokens_count)
{
/*    String* tokens = NULL;
    int tokens_count = 0;*/

    Token* tokens = NULL;
    int tokens_count = 0;

    String current_token = {0};
    for (int i = 0; i < strlen(text); i++)
    {
        switch (text[i])
        {
            case ' ':
            case '\n':
            case '(':
            case ')':
            {
 
                if (current_token.text != NULL)
                {
                    tokens_count++;
                    if (tokens)
                    {
                        tokens = realloc(tokens, sizeof(Token) * tokens_count);
                        memset(tokens + (tokens_count-1), 0, sizeof(Token));
                    }
                    else
                    {
                        tokens = calloc(tokens_count, sizeof(Token));
                    }
                    tokens[tokens_count-1].start_index = i - current_token.len;
                    tokens[tokens_count-1].end_index = i;
                    String_set(&(tokens[tokens_count-1].text), current_token.text);

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

                                if (strcmp(current_token.text, keyword_start) == 0)
                                {
                                    tokens[tokens_count-1].type = TOKEN_KEYWORD;
                                    break;
                                }

                                if (j != keywords_len)
                                {
                                    keywords_copy[j] = ':';
                                    keyword_start = keywords_copy + j + 1;
                                }
                            }
                        }
                        free(keywords_copy);
                    }

                    String_clear(&current_token);
                }

                //add the delimiter as a TOKEN_NONE if needed
                if ((text[i] != ' ') && (text[i] != '\n'))
                {
                    tokens_count++;
                    if (tokens)
                    {
                        tokens = realloc(tokens, sizeof(Token) * tokens_count);
                        memset(tokens + (tokens_count-1), 0, sizeof(Token));
                    }
                    else
                    {
                        tokens = calloc(tokens_count, sizeof(Token));
                    }

                    tokens[tokens_count-1].start_index = i; 
                    tokens[tokens_count-1].end_index = i;
                    tokens[tokens_count-1].type = TOKEN_NONE;
                    String_push(&(tokens[tokens_count-1].text), text[i]);
                }
            } break;

            default:
            {
                String_push(&current_token, text[i]);
            } break;
        }
    }

    for (int i = 0; i < tokens_count; i++)
    {
        printf("'%s', %d, %d, %d\n", tokens[i].text.text,
        tokens[i].start_index, tokens[i].end_index, tokens[i].type);
    }
    printf("\n");

    *out_tokens_count = tokens_count;
    return tokens;
}
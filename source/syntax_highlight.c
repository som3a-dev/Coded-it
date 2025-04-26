#include <stdlib.h>
#include <memory.h>
#include "string.h"

const char* keywords = "auto:break:case:char:const:continue:default:do:double:else:enum:extern:float:for:goto:if:inline:int:long:register:restrict:return:short:signed:sizeof:static:struct:switch:typedef:union:unsigned:void:volatile:while:_Bool:_Complex:_Imaginary";


void parse(const char* text)
{
    String* tokens = NULL;
    int tokens_count = 0;

    String current_token = {0};
    for (int i = 0; i < strlen(text); i++)
    {
        switch (text[i])
        {
            case ' ':
            case '\n':
            {
                if (current_token.text != NULL)
                {
                    tokens_count++;
                    if (tokens)
                    {
                        tokens = realloc(tokens, sizeof(String) * tokens_count);
                        memset(tokens + (tokens_count-1), 0, sizeof(String));
                    }
                    else
                    {
                        tokens = calloc(tokens_count, sizeof(String));
                    }

                    String_set(tokens + (tokens_count-1), current_token.text);

                    String_clear(&current_token);
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
        printf("%s\n", tokens[i].text);
    }
}
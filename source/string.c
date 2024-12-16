#include "string.h"



void String_push(String* str, char c)
{
    if (!str) return;

    if (str->text)
    {
        str->len++;
        str->text = realloc(str->text, sizeof(char) * str->len+1);
        str->text[str->len-1] = c;
        str->text[str->len] = 0;
    }
    else
    {
        str->len = 1;
        str->text = malloc(sizeof(char) * str->len+1);
        str->text[0] = c;
        str->text[1] = '\0';
    }
}


void String_pop(String* str)
{
    if (!str) return;
    if (!(str->text)) return;
    if (str->len <= 0) return;

    str->text[str->len-1] = '\0';
    str->len--;
    str->text = realloc(str->text, sizeof(char) * str->len+1);
}

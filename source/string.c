#include "string.h"



void String_add(String* str, char c)
{
    if (!str) return;

    if (str->text)
    {
        str->len++;
        str->text = realloc(str->text, sizeof(char) * str->len);
        str->text[str->len-2] = c;
        str->text[str->len-1] = 0;
    }
    else
    {
        str->len = 2;
        str->text = malloc(sizeof(char) * str->len);
        str->text[0] = c;
        str->text[1] = '\0';
    }
}

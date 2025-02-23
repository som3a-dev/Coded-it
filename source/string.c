#include "string.h"

#include <stdlib.h>
#include <string.h>



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

//O(n) worst case
void String_insert(String* str, char c, int index)
{
    if (!str) return;
    if (index < 0) return;
    if (index > str->len) return;

    if (!(str->text))
    {
        str->len = 1;
        str->text = malloc(str->text, sizeof(char) * str->len + 1);
    }
    else
    {
        str->len++;
        str->text = realloc(str->text, sizeof(char) * str->len + 1);
    }

    str->text[str->len] = '\0';

    for (int i = str->len-2; i >= index; i--)
    {
        str->text[i + 1] = str->text[i];
    }

    str->text[index] = c;
}


void String_remove(String* str, int index)
{
    if (!str) return;
    if (index < 0) return;
    if (index > str->len - 1) return;

    for (int i = index; i < str->len-1; i++)
    {
        str->text[i] = str->text[i + 1];
    }

    str->len--;
    str->text = realloc(str->text, sizeof(char) * str->len + 1);
    str->text[str->len] = '\0';
}


int String_get_previous_newline(String* str, int index)
{
    if (!str) return -2;
    if (!str->len) return -3;
    if (!str->text) return -4;
    for (int i = index-1; i >= 0; i--)
    {
        if (str->text[i] == '\n')
        {
            return i;
        }
    }

    return -1; //no newline found
}


int String_get_next_newline(String* str, int index)
{
    if (!str) return -2;
    if (!str->len) return -3;
    if (!str->text) return -4;
    for (int i = index + 1; i < str->len; i++)
    {
        if (str->text[i] == '\n')
        {
            return i;
        }
    }

    return str->len; //no newline found
}


void String_clear(String* str)
{
    if (!str) return;

    if (str->text)
    {
        free(str->text);
        str->text = NULL;
    }

    str->len = 0;
}


void String_set(String* str, const char* text)
{
    if (!str) return;

    String_clear(str);

    for (int i = 0; i < strlen(text); i++)
    {
        String_push(str, text[i]);
    }
}
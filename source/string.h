#pragma once

typedef struct
{
    char* text;
    int len;
} String;



void String_push(String* str, char c);

void String_pop(String* str);

void String_insert(String* str, char c, int index);

void String_remove(String* str, int index);


//TODO(omar): im not sure if this function should be here as part of the String class.
// 
//Gets the index of the newline right before the character at index.
//"Hello\nWorld\n!"
//if you give the index of 'W' it will return 5
//if you give the index of '!' it will return 11
//if you give the index of 'H' it will return -1 (no newline exists before 'H')
int String_get_previous_newline(String* str, int index);

//same stuff. but
//if no newline is found we return str->len instead of -1.
int String_get_next_newline(String* str, int index);

void String_clear(String* str);

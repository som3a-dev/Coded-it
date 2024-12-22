

typedef struct
{
    char* text;
    int len;
} String;



void String_push(String* str, char c);
void String_pop(String* str);
void String_insert(String* str, char c, int index);
void String_remove(String* str, int index);

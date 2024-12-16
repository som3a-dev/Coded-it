

typedef struct
{
    char* text;
    int len;
} String;



void String_push(String* str, char c);
void String_pop(String* str);

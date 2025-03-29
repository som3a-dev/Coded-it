/*GENERAL PURPOSE STACK*/
#pragma once

typedef struct
{
    char* arr;
    int len;
    int element_size;
} Stack;



void Stack_init(Stack* stack, size_t element_size);
void Stack_push(Stack* stack, char* element);

//NOTE(omar): THIS FUNCTION RETURNS A COPY OF THE LAST ELEMENT.
//            MAKE SURE TO FREE() AFTER YOU ARE DONE
char* Stack_pop(Stack* stack, bool return_element); 

void Stack_clear(Stack* stack);
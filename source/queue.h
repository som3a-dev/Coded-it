#pragma once
#include <stdbool.h>

typedef struct
{
    char* arr;
    int len;
    size_t element_size;
} Queue;



void Queue_init(Queue* queue, size_t element_size);
void Queue_push(Queue* queue, char* element);

//NOTE(omar): THIS FUNCTION RETURNS A COPY OF THE LAST ELEMENT.
//            MAKE SURE TO FREE() AFTER YOU ARE DONE
char* Queue_pop(Queue* queue, bool return_element); 

void Queue_clear(Queue* queue);


//for testing
void Queue_print(Queue* queue);

#include <memory.h>
#include <stdlib.h>
#include "queue.h"

void Queue_init(Queue* queue, size_t element_size)
{
    memset(queue, 0, sizeof(Queue));

    queue->element_size = element_size;
}


void Queue_push(Queue* queue, char* element)
{
    if (!queue) return;
    if (queue->element_size == 0) return;

    if (queue->arr)
    {
        queue->len++;
        queue->arr = realloc(queue->arr, queue->element_size * queue->len);
        
        char* element_start = queue->arr + (queue->len-1) * queue->element_size;
        memcpy(element_start, element, queue->element_size);
    }
    else
    {
        queue->len = 1;
        queue->arr = malloc(queue->element_size);
        memcpy(queue->arr, element, queue->element_size);
    }
}


char* Queue_pop(Queue* queue, bool return_element)
{
    if (!queue) return NULL;
    if (queue->element_size == 0) return NULL;
    if (!(queue->arr)) return NULL;
    if (queue->len == 0) return NULL;

    char* copy = NULL;
    if (return_element)
    {
        char* element = queue->arr;
        copy = malloc(queue->element_size);
        memcpy(copy, element, queue->element_size);
    }

    for (int i = 0; i < (queue->len - 1); i++)
    {
        char* element = queue->arr + (i * queue->element_size);
        char* next = queue->arr + ((i + 1) * queue->element_size);

        memcpy(element, next, queue->element_size);
    }

    queue->len--;
    queue->arr = realloc(queue->arr, queue->len * queue->element_size);

    return copy;
}


void Queue_clear(Queue* queue)
{
    if (!queue) return;
    
    if (queue->arr)
    {
        free(queue->arr);
        queue->arr = NULL;
    }

    queue->len = 0;
}


void Queue_print(Queue* queue)
{
    if (!queue) return;
    for (int i = 0; i < queue->len; i++)
    {
        printf("%hhu", queue->arr[i]);
        if (i != (queue->len-1))
        {
            printf(", ");
        }
    } 

    printf("\n");
}







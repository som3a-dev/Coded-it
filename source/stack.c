#include <memory.h>
#include <stdlib.h>
#include "stack.h"


void Stack_init(Stack* stack, size_t element_size)
{
    memset(stack, 0, sizeof(Stack));

    stack->element_size = element_size;
}


void Stack_push(Stack* stack, char* element)
{
    if (!stack) return;
    if (stack->element_size == 0) return;

    if (stack->arr)
    {
        stack->len++;
        stack->arr = realloc(stack->arr, stack->element_size * stack->len);
        
        char* element_start = stack->arr + (stack->len-1) * stack->element_size;
        memcpy(element_start, element, stack->element_size);
    }
    else
    {
        stack->len = 1;
        stack->arr = malloc(stack->element_size);
        memcpy(stack->arr, element, stack->element_size);
    }
}


char* Stack_pop(Stack* stack, bool return_element)
{
    if (!stack) return NULL;
    if (stack->element_size == 0) return NULL;
    if (!(stack->arr)) return NULL;
    if (stack->len == 0) return NULL;

    char* copy = NULL;
    if (return_element)
    {
        char* element = stack->arr + (stack->len-1) * stack->element_size;
        copy = malloc(stack->element_size);
        memcpy(copy, element, stack->element_size);
    }

    stack->len--;
    if (stack->len > 0)
    {
        stack->arr = realloc(stack->arr, stack->len * stack->element_size);
    }
    else
    {
        free(stack->arr);
        stack->arr = NULL;
    }

    return copy;
}


char* Stack_get(Stack* stack, int index)
{
   if (!stack) return NULL;
   if (index >= stack->len) return NULL;
   if (index < 0) return NULL;

    char* element = stack->arr + (index * stack->element_size);
    return element;
}


void Stack_clear(Stack* stack)
{
    if (!stack) return;
    
    if (stack->arr)
    {
        free(stack->arr);
        stack->arr = NULL;
    }

    stack->len = 0;
}


void Stack_print(Stack* stack)
{
    if (!stack) return;
    for (int i = 0; i < stack->len; i++)
    {
        printf("%hhu", stack->arr[i]);
        if (i != (stack->len-1))
        {
            printf(", ");
        }
    } 

    printf("\n");
}




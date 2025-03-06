#include "array.h"

#include <stdbool.h>



void ArrayInt_push(ArrayInt* array, int val)
{
    if (!array) return;
    
    if (array->elements)
    {
        array->count++;
        array->elements = realloc(array->elements, sizeof(int) * array->count);
        array->elements[array->count-1] = val;
    }
    else
    {
        array->count = 1;
        array->elements = malloc(sizeof(int) * array->count);
        array->elements[0] = val;
    }
}


int ArrayInt_pop(ArrayInt* array)
{
    if (!array) return;
    if (!(array->elements)) return;
    if (array->count <= 0) return;
    
    int element = array->elements[array->count-1];

    array->count--;
    array->elements = realloc(array->elements, sizeof(int) * array->count);

    return element;
}


bool ArrayInt_get(ArrayInt* array, int index, int* out)
{
    if (!array) return false;
    if (!(array->elements)) return false;
    if ((array->count <= index) || (index < 0)) return false;
    
    *out = array->elements[index]; 
    return true;
}


void ArrayInt_remove(ArrayInt* array, int val)
{
    if (!array) return;

    int index = -1;
    for (int i = 0; i < array->count; i++)
    {
        if (array->elements[i] == val)
        {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    for (int i = index; i < array->count; i++)
    {
        array->elements[i] = array->elements[i + 1];
    }

    array->count--;
    array->elements = realloc(array->elements, sizeof(int) * array->count);
}








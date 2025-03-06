#pragma once

#include <stdbool.h>

typedef struct
{
    int* elements;
    int count;
} ArrayInt;


void ArrayInt_push(ArrayInt* array, int val);
int ArrayInt_pop(ArrayInt* array);

bool ArrayInt_get(ArrayInt* array, int index, int* out);
void ArrayInt_remove(ArrayInt* array, int val);

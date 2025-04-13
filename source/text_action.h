#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "string.h"


enum
{
    TEXT_ACTION_NONE,
    TEXT_ACTION_WRITE,
    TEXT_ACTION_REMOVE,
    TEXT_ACTION_COUNT
};


typedef struct
{
    int type;
    String text; //used if the text was more than 1 character
    char character; //used if the text was 1 character

    int start_index; //the index at which the action happened in the file's text 
} TextAction;


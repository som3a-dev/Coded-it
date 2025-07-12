#pragma once

#include "hash_table.h"


enum
{
    JSON_TOKEN_NONE,
    JSON_TOKEN_STRING,
    JSON_TOKEN_INT,
    JSON_TOKEN_BOOL,
    JSON_TOKEN_CHAR
};

//THE ENUM OF JSON_TOKEN MUST BE A SUBSET OF JSON_VALUE THAT IS ALWAYS
//AT THE START AND IN THE SAME ORDER IN JSON_VALUE
//NOT ABIDING BY THAT CAN AND WILL BREAK BEHAVIOR FOR NOW
enum
{
    JSON_VALUE_NONE,
    JSON_VALUE_STRING,
    JSON_VALUE_INT,
    JSON_VALUE_BOOL,
    JSON_VALUE_CHAR,
    JSON_VALUE_OBJECT,
    JSON_VALUE_ARRAY
};


typedef struct
{
    int type;

    //this is a real allocated pointer only in the case of strings
    //in other cases (int, char) this is an encoded pointer
    char* val;
} json_token;

typedef struct
{
    int type;

    //this is a real allocated pointer only in the case of strings
    //in other cases (int, char) this is an encoded pointer
    char* val;
} json_value;


typedef struct
{
    json_value* values;
    int values_count;
} json_array; //an array of json_values. Also considered a json_value

typedef struct
{
    hash_table table; //a hash table of json_value
} json_object;



json_object* jp_parse_file(const char* path);

void json_value_print(const json_value* val);
void json_token_print(const json_token* token);

void json_token_destroy(json_token* token);
void json_value_destroy(json_value* val);

json_value* jp_get_child_value_in_object(const json_object* object, const char* path);












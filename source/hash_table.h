#pragma once
#include <stdint.h>


typedef struct
{
    uint64_t* key_hashes; //the fnv hashes of each key.
                     //NOT the index (which is hash % table->len)

    char* vals; //the values
    int len;
    size_t element_size; //NEVER CHANGE on your own
} hash_table;



//MUST BE CALLED
void hash_table_init(hash_table* table, int initial_len,
                     size_t element_size);

//NOTE(omar): doesn't change element size to zero
//but frees vals and key_hashes and sets len to 0
void hash_table_clear(hash_table* table);

void hash_table_set(hash_table* table, const char* key,
                    const char* val);

//returns a reference to the element in the hash table's
//internal valsay. not a copy of the element
char* hash_table_get(const hash_table* table, const char* key);

char* hash_table_get_by_index(const hash_table* table, int index);

void hash_table_print(const hash_table* table);

void _hash_table_resize(hash_table* table, int new_len);


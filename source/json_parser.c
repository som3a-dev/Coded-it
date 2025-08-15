#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "json_parser.h"
#include "string.h"
#include "hash_table.h"


#define IS_OPEN_BRACKET(x) ((x == '{') || (x == '['))
#define IS_CLOSED_BRACKET(x) ((x =='}') || (x == ']'))
#define IS_CURLY_BRACKET(x) ((x == '{') || (x == '}'))
#define IS_BRACKET(x) (IS_OPEN_BRACKET(x) || IS_CLOSED_BRACKET(x))


static void json_array_push(json_array* array, const json_value* val);
static json_value* json_array_get(const json_array* array, int index);


//use them for json_value too its alr
static inline void json_token_set_int(json_token* token, int val);
static inline void json_token_set_string(json_token* token, char* val);
static inline void json_token_set_bool(json_token* token, bool val);
static inline void json_token_set_char(json_token* token, char val);



//helper for jp_lex()
static void _resize_tokens_array(json_token** tokens, int* tokens_count, int new_count);


bool is_index_part_of_literal(const char* text, int index);

json_object* jp_parse(json_token* tokens, const int tokens_count);

//token is the token of the '[' character of the array
json_array* jp_parse_array(json_token** token, const int tokens_count,
                    int* token_index);

json_object* jp_parse_object(json_token** token, const int tokens_count, int* token_index);

int jp_parse_key_value(json_token** tokens, const int tokens_count,
                        int* token_index, hash_table* json_values);


json_token* jp_lex(const char* str, int* out_tokens_count);

String jp_lex_string(const char* str);

String jp_lex_numeric(const char* str);

//str isn't changed after the function is done. it is mutated in the function
//and then reset back to its original state
//so it is practically const
String jp_lex_bool(char* str);


json_value* jp_get_child_value_in_object(const json_object* object, const char* path)
{
    int len = strlen(path);
    char* pathcpy = malloc(sizeof(char) * (len+1));
    memcpy(pathcpy, path, len+1);

    for (int i = 0; i < len; i++)
    {
        if ((pathcpy[i] == '/') || (pathcpy[i] == '\\'))
        {
            pathcpy[i] = '\0';
        }
    }

    json_value* val = NULL;
    hash_table* table = &(object->table);
    char* str = pathcpy;
    for (int i = 0; i <= len; i++)
    {
        if (pathcpy[i] == '\0')
        {
            if (table == NULL)
            {
                //previous value in the tree was not an object. can't keep going
                printf("ERROR: Trying to traverse into a non object json value!\n");
                goto end;
            }

            val = hash_table_get(table, str);
            if (val == NULL)
            {
                printf("ERROR: Invalid path for json value\n");
                goto end;
            }

            //if val is an object. set table to its table to be able to traverse next layer in the path if needed
            //or if we try to traverse an extra layer and the previous value was not an object. we can freak out
            table = NULL;
            if (val->type == JSON_VALUE_OBJECT)
            {
                json_object* val_obj = (json_object*)val->val;
                assert(val_obj);

                table = &(val_obj->table);
            }

            printf("%s|", str);
            str = pathcpy + i + 1;
        }
    }

    end:
    printf("\n");
    free(pathcpy);
    return val;
}


json_object* jp_parse_file(const char* path)
{
    FILE* fp;
    fopen_s(&fp, path, "rb");

    if (!fp)
    {
        printf("JSON PARSER: invalid file path '%s'\n", path);
        return NULL;
    }

    char* file_string = NULL;
    long file_len = 0;

    fseek(fp, 0L, SEEK_END);
    file_len = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    file_string = malloc(sizeof(char) * (file_len+1));
    fread_s(file_string, sizeof(char) * file_len, sizeof(char), file_len, fp);
    file_string[file_len] = '\0';

    //lexing
    int tokens_count;
    json_token* tokens = jp_lex(file_string, &tokens_count);
    json_object* json_obj = jp_parse(tokens, tokens_count);

/*    hash_table* json_values = &(json_obj->table);
    json_value* val = hash_table_get(json_values, "semanticTokenColors");

    json_object* obj = (json_object*)(val->val);
    json_value* number_literal_color = hash_table_get(&(obj->table), "numberLiteral");
    json_value_print(number_literal_color);*/

    free(tokens);
    fclose(fp);
    free(file_string);

    return json_obj;
}


//NOTE(omar): right now we use the same json_token->val for the
//json_object. maybe consider making copies of the vals instead
//if needed. 
json_object* jp_parse(json_token* tokens, const int tokens_count)
{
    if (tokens == NULL)
    {
        return NULL;
    }

    int i = 0;
    // if (tokens->type == JSON_TOKEN_CHAR)
    {
        if (tokens->val == '{')
        {
            json_object* obj = jp_parse_object(&tokens, tokens_count, &i);
            return obj;
        }
    }

    printf("ERROR: Expected object at the start of the json file");
    return NULL;
}



json_array* jp_parse_array(json_token** token, const int tokens_count,
                    int* token_index)
{
    json_array* array = calloc(1, sizeof(json_array));

    //to get the next token after the '['
    json_token* prev_token = *token;

    (*token)++;
    (*token_index)++;

    for (; (*token_index) < tokens_count; (*token_index)++)
    {
        json_token* t = (*token);
        
        if ((t->type == JSON_TOKEN_CHAR) && (t->val == ','))
        {
            if (prev_token->type == JSON_TOKEN_CHAR)
            {
                if (IS_CLOSED_BRACKET(prev_token->val) == false)
                {
                    printf("\n\nERROR: Unexpected character '%c', token index: %d\n\n", t->val, *token_index);
                    free(array);
                    return NULL;
                }
            }
            goto next_token;
        }
        if ((t->type == JSON_TOKEN_CHAR) && (t->val == ']'))
        {
            break;
        }

        if (prev_token->type != JSON_TOKEN_CHAR)
        {
            free(array);
            return NULL;
        }
        if ((prev_token->val != '[') && (prev_token->val != ','))
        {
            printf("\n\nERROR: Unexpected character '%c', token index: %d\n\n", t->val, *token_index);
            free(array);
            return NULL;
        }

        if (t->type == JSON_TOKEN_CHAR)
        {
            if (t->val == '[')
            {
                json_array* sub_array = jp_parse_array(token, tokens_count, token_index);
                if (sub_array == NULL)
                {
                    free(array);
                    return NULL;
                }
                json_value array_val = {JSON_VALUE_ARRAY, sub_array};
                json_array_push(array, &array_val);
            }
            else if (t->val == '{')
            {
                json_object* object = jp_parse_object(token, tokens_count, token_index);
                if (object == NULL)
                {
                    free(array);
                    return NULL;
                }
                json_value object_val = {JSON_VALUE_OBJECT, object};
                json_array_push(array, &object_val);
            }
            else
            {
                printf("\n\nERROR: Unexpected character '%c', token index: %d\n\n", t->val, *token_index);
                free(array);
                return NULL;
            }
        }
        else
        {
            json_array_push(array, t);
        }

        next_token:
        prev_token = (*token);
        (*token)++;
    }

    return array;
}


json_object* jp_parse_object(json_token** token, const int tokens_count, int* token_index)
{
    json_object* obj = calloc(1, sizeof(json_object));
    hash_table_init(&(obj->table), 0, sizeof(json_value));

    json_token* prev_token = *token;

    (*token)++;
    (*token_index)++;

    json_token* t;
    for (; (*token_index) < tokens_count; (*token_index)++)
    {
        t = (*token);

        switch (t->type)
        {
            case JSON_TOKEN_CHAR:
            {
                if (t->val == '}')
                {
                    if (prev_token->type == JSON_TOKEN_CHAR)
                    {
                        if (IS_CURLY_BRACKET(prev_token->val) == false &&
                        (prev_token->val != ','))
                        {
                            printf("\n\nERROR: Unexpected character '%c', token index: %d\n\n", t->val, *token_index);
                            free(obj);
                            return NULL;
                        }
                    }
                    goto end;
                }
                if (t->val == ',')
                {
                    goto next_token;
                }
                
                printf("\n\nERROR: Unexpected character '%c', token index: %d\n\n", t->val, *token_index);
                free(obj);
                return NULL;
            } break;

            case JSON_TOKEN_STRING:
            {
                if (prev_token->type != JSON_TOKEN_CHAR)
                {
                    printf("ERROR %d: Invalid token", __LINE__);
                    free(obj);
                    return NULL;
                }

                if ((prev_token->val != '{') && (prev_token->val != ','))
                {
                    printf("ERROR %d: Invalid token", __LINE__);
                    free(obj);
                    return NULL;
                }

                if (jp_parse_key_value(token, tokens_count, token_index, &(obj->table)))
                {
                    free(obj);
                    return NULL;
                }
            } break;

            default:
            {
                printf("ERROR %d: Invalid token", __LINE__);
                free(obj);
                return NULL;
            } break;
        }
        
        next_token:
        prev_token = t;
        (*token)++;
    }

    end:

    return obj;
}


int jp_parse_key_value(json_token** token, const int tokens_count,
                        int* token_index, hash_table* json_values)
{
    if ((*token_index) == (tokens_count-1))
    {
        //TODO(omar):
        //There is no next token ? print an error
        printf("ERROR %d: No next token\n", __LINE__);
        return 1;
    }

    json_token* key = *token;

    (*token)++;
    (*token_index)++;
    if ((*token)->type != JSON_TOKEN_CHAR)
    {
        //TODO(omar):
        //Next token is not a character? print an error
        printf("ERROR %d: Invalid token", __LINE__);
        return 2;
    }
    
    if ((*token)->val == ':')
    {
        if ((*token_index) == (tokens_count-1))
        {
            //TODO(omar):
            //Token has no value token ? print an error
            printf("ERROR %d: Invalid token", __LINE__);
            return 3;
        }

        (*token)++;
        (*token_index)++;

        switch ((*token)->type)
        {
            case JSON_TOKEN_CHAR:
            {
                switch ((char)((*token)->val))
                {
                    case '[':
                    {
                        json_array* array = jp_parse_array(token, tokens_count,
                        token_index);

                        json_value array_val = {JSON_VALUE_ARRAY, array};

                        String key_str = {0};
                        String_set(&key_str, key->val);
                        String_remove(&key_str, key_str.len-1, NULL);
                        String_remove(&key_str, 0, NULL);

                        hash_table_set(json_values, key_str.text, &array_val);
                        String_clear(&key_str);
                    } break;

                    case '{':
                    {
                        json_object* obj = jp_parse_object(token, tokens_count, token_index);

                        json_value obj_val = {JSON_VALUE_OBJECT, obj};

                        String key_str = {0};
                        String_set(&key_str, key->val);
                        String_remove(&key_str, key_str.len-1, NULL);
                        String_remove(&key_str, 0, NULL);

                        hash_table_set(json_values, key_str.text, &obj_val);
                        String_clear(&key_str);
                    } break;

                    default:
                    {
                        printf("ERROR %d: Invalid token", __LINE__);
                        return 2;
                    } break;
                }
            } break;

            default:
            {
                String key_str = {0};
                String_set(&key_str, key->val);
                String_remove(&key_str, key_str.len-1, NULL);
                String_remove(&key_str, 0, NULL);

                hash_table_set(json_values, key_str.text, *token);
                String_clear(&key_str);

            } break;
        }
    }
    else
    {
        printf("ERROR %d: Invalid token", __LINE__);
        return 2;
    }
    
    return 0;
}


json_token* jp_lex(const char* str, int* out_tokens_count)
{
    json_token* tokens = NULL;
    int tokens_count = 0;

    //used for debugging and errors
    int char_index = 0;

    while (str[0])
    {
        String json_str = jp_lex_string(str);
        if (json_str.text)
        {
            str += json_str.len;
            char_index += json_str.len;
            
            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            //DON'T CLEAR THE json_str.
            //json_token_set_string doesn't copy the string. it just uses the existing one in json_str
            //the string should be freed later
            json_token_set_string(tokens + (tokens_count-1), json_str.text);

//            printf("%s\n", tokens[tokens_count-1].val);
            continue;
        }

        String json_numeric = jp_lex_numeric(str);
        if (json_numeric.text)
        {
            str += json_numeric.len;
            char_index += json_numeric.len;

            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            int json_numeric_value = atoi(json_numeric.text);
            json_token_set_int(tokens + (tokens_count-1), json_numeric_value);

            String_clear(&json_numeric);
            continue;
        }

        String json_bool = jp_lex_bool(str);
        if (json_bool.text)
        {
//            printf("%s\n", json_bool.text);
            str += json_bool.len;
            char_index += json_bool.len;

            _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);

            bool json_bool_value = false;
            if (strcmp(json_bool.text, "true") == 0)
            {
                json_bool_value = true;
            }
            json_token_set_bool(tokens + (tokens_count-1), json_bool_value);

            String_clear(&json_bool);
            continue;
        }

        switch (str[0])
        {
            case ':':
            case '[':
            case ']':
            case '{':
            case '}':
            case ',':
            {
//                printf("%c\n", str[0]);
                _resize_tokens_array(&tokens, &tokens_count, tokens_count+1);
                json_token_set_char(tokens + (tokens_count-1), str[0]);
            } break;

            case '\n':
            case '\r':
            case '\t':
            case ' ':
            {

            } break;

            default:
            {
                printf("\n\nERROR: Unexpected character '%c', index: %d\n\n", str[0], char_index);
                for (int i = 0; i < tokens_count; i++)
                {
                    json_token_destroy(tokens + i);
                }
                free(tokens);
                return NULL;
            } break;
        }
        
        str++;
        char_index++;
    }

/*    for (int i = 0; i < tokens_count; i++)
    {
        json_token_print(tokens + i);
        printf(" | ");
    }
    printf("\n");*/

    if (out_tokens_count)
    {
        *out_tokens_count = tokens_count;
    }

    return tokens;
}


String jp_lex_string(const char* str)
{
    String result = {0};
    int len = strlen(str);

    if (str[0] == '"')
    {
        String_push(&result, '"');
        for (int i = 1; i < len; i++)
        {
            if (str[i] == '"')
            {
                break;
            }

            String_push(&result, str[i]);
        }
        String_push(&result, '"');
    }

    return result;
}


String jp_lex_numeric(const char* str)
{
    String number_string = {0}; //the string containing the number we lexed
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        if (str[i] < '0') break;
        if (str[i] > '9') break;

        String_push(&number_string, str[i]);
    }

    return number_string;
}


String jp_lex_bool(char* str)
{
    String bool_string = {0};
    int len =  strlen(str);

    //check for true
    if (len >= 4)
    {
        char temp = str[4];
        str[4] = '\0';

        if (strcmp(str, "true") == 0)
        {
            String_set(&bool_string, "true");
            str[4] = temp;
            return bool_string;
        }

        str[4] = temp;
    }

    //check for false
    if (len >= 5)
    {
        char temp = str[5];
        str[5] = '\0';

        if (strcmp(str, "false") == 0)
        {
            String_set(&bool_string, "false");
            str[5] = temp;
            return bool_string;
        }

        str[5] = temp;
    }

    return bool_string;
}


static void _resize_tokens_array(json_token** tokens, int* tokens_count, int new_count)
{
    if (new_count < 0) return;
    if (new_count == *tokens_count) return;

    if ((*tokens))
    {
        (*tokens) = realloc(*tokens, new_count * sizeof(json_token));

        if (new_count > (*tokens_count))
        {
            json_token* new_memory = (*tokens) + (*tokens_count);
            memset(new_memory, 0, sizeof(json_token) * (new_count - (*tokens_count)));
        }
    }
    else
    {
        (*tokens) = calloc(new_count, sizeof(json_token));
    }

    *tokens_count = new_count;
}


static inline void json_token_set_int(json_token* token, int val)
{
    token->type = JSON_TOKEN_INT;
    token->val = val;
}


static inline void json_token_set_string(json_token* token, char* val)
{
    token->type = JSON_TOKEN_STRING;
    token->val = val;
}


static inline void json_token_set_bool(json_token* token, bool val)
{
    token->type = JSON_TOKEN_BOOL;
    token->val = val;
}


static inline void json_token_set_char(json_token* token, char val)
{
    token->type = JSON_TOKEN_CHAR;
    token->val = (char *)(uintptr_t)(unsigned char)val;
}


void json_value_print(const json_value* val)
{
    if (val == NULL) return;

    switch (val->type)
    {
        case JSON_VALUE_ARRAY:
        {
            json_array* arr = (json_array*)val->val;
            printf("[");
            for (int i = 0; i < arr->values_count; i++)
            {
                json_value_print(arr->values + i);
                if (i != ((arr->values_count)-1))
                {
                    printf(", ");
                }
            }
            printf("]");
        } break;

        case JSON_VALUE_OBJECT:
        {
            hash_table* table = (hash_table*)val->val;

            for (int i = 0; i < table->count; i++)
            {
                char* key = hash_table_get_key(table, i);
                char* val = hash_table_get(table, key);
                printf("%s: ", key);
                json_value_print(val);
                printf("\n");
            }
            printf("\n");
        } break;

        default:
        {
            json_token_print(val);
        } break;
    }
}


void json_token_print(const json_token* token)
{
    if (token == NULL) return;
    switch (token->type)
    {
        case JSON_TOKEN_STRING:
        {
            printf("%s", token->val);
        } break;

        case JSON_TOKEN_INT:
        {
            printf("%d", token->val);
        } break;

        case JSON_TOKEN_BOOL:
        {
            const char* str = "true";
            if (token->val == 0)
            {
                str = "false";
            }
            printf("%s", str);
        } break;

        case JSON_TOKEN_CHAR:
        {
            printf("%c", token->val);
        } break;
    }
}


void json_token_destroy(json_token* token)
{
    if (token->type == JSON_TOKEN_STRING)
    {
        if (token->val)
        {
            free(token->val);
            token->val = NULL;
        }
    }
}


void json_value_destroy(json_value* val)
{
    switch (val->type)
    {
        case JSON_VALUE_ARRAY:
        {
            json_array* array = (json_array*)(val->val);
            for (int i = 0; i < array->values_count; i++)
            {
                json_value_destroy(array->values + i);
            }

            free(val->val);
            val->val = NULL;
        } break;

        case JSON_VALUE_OBJECT:
        {
//            assert(false && "Desturction of json value of type JSON_VALUE_OBJECT not yet implemented\n");
            json_object* obj = (json_object*)(val->val);
            for (int i = 0; i < obj->table.count; i++)
            {
                json_value* val = hash_table_get(&(obj->table),
                hash_table_get_key(&(obj->table), i));

                json_value_destroy(val);
            }
        } break;

        default:
        {
            json_token_destroy(val);
        } break;
    }
}


static void json_array_push(json_array* array, const json_value* val)
{
    //NOTE(omar): THIS IS FINE AS LONG AS json_token and json_value
    //are the same thing (a int type and char* val)
    //this shit will all be rewritten if they ever have to 
    //stop being interchangable
    _resize_tokens_array(&(array->values), &(array->values_count),
    (array->values_count)+1);

    json_value* last_val = array->values + (array->values_count-1);
    memcpy(last_val, val, sizeof(json_value));
}


static json_value* json_array_get(const json_array* array, int index)
{
    if (index >= array->values_count)
    {
        return NULL;
    }

    return array->values + index;
}


bool is_index_part_of_literal(const char* text, int index)
{
    int quote_count = 0;
    for (int i = 0; i < index; i++)
    {
        if (text[i] == '"')
        {
            quote_count++;
        }
    }

    return ((quote_count % 2) != 0);
}











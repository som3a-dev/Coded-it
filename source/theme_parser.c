#include "theme_parser.h"
#include <stdbool.h>
#include <assert.h>

json_value* tp_get_color_in_token_colors(json_array* token_colors, const char* token_str)
{
    json_value* token_color = NULL;
    
    for (int i = 0; i < token_colors->values_count; i++) 
    {
        json_value* val = token_colors->values + i;
        if (val->type != JSON_VALUE_OBJECT)
        {
            //TODO(omar): this probably should never happen. maybe print an error
            assert(false);
            continue;
        }

        json_object* obj = (json_object*)(val->val);
        assert(obj);

        json_value* scope = hash_table_get(&(obj->table), "scope");
        if (scope == NULL)
        {
            //TODO(omar): this probably should never happen. maybe print an error
            assert(false);
            continue;
        }

        if (scope->type == JSON_VALUE_STRING)
        {
            if (strcmp(scope->val, token_str)== 0)
            {
                json_value* settings = hash_table_get(&(obj->table), "settings");
                assert(settings);
                json_object* settings_obj = (json_object*)(settings->val);

                token_color = hash_table_get(&(settings_obj->table), "foreground");
                return token_color;
            }
        }
        else if (scope->type == JSON_VALUE_ARRAY)
        {
            json_array* arr = (json_array*)(scope->val);
            for (int i = 0; i < arr->values_count; i++)
            {
                json_value* val = arr->values + i;
                if (val->type == JSON_VALUE_STRING)
                {
                    if (strcmp(val->val, token_str) == 0)
                    {
                        json_value* settings = hash_table_get(&(obj->table), "settings");
                        assert(settings);
                        json_object* settings_obj = (json_object*)(settings->val);

                        token_color = hash_table_get(&(settings_obj->table), "foreground");
                        return token_color;
                    }
                }
            }
        }
    }

    return token_color;
}
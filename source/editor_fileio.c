#include "editor_fileio.h"
#include <windows.h>

int editor_save_file(const ProgramState* state, const char* filename)
{
    int return_code = FILEIO_PATH_WAS_INVALID;

    if (filename == NULL)
    {
        printf("No current open file.\n");
        return return_code;
    }

    char* msg_format = NULL;
    FILE* fp;
    String filepath = {0};

    String_insert_string(&filepath, filename, 0);
    String_insert(&filepath, '\\', 0);
    String_insert_string(&filepath, state->current_directory.text, 0);
    fopen_s(&fp, filepath.text, "w");

    if (!fp)
    {
        int attributes = GetFileAttributes(filepath.text);
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            String_set(&(state->current_directory), filepath.text);
            return_code = FILEIO_PATH_WAS_DIRECTORY;
        }
        else
        {

            msg_format = "Couldn't save to file '%s'.";
        }
    }
    else
    {
        //fucking text.text.text
        fwrite(state->text.text.text, sizeof(char), state->text.text.len, fp);
        fclose(fp);

        msg_format = "Saved to file '%s'.";
        return_code = FILEIO_PATH_WAS_FILE;
    }

    if (msg_format)
    {
        size_t msg_size = sizeof(char) * (strlen(msg_format) + strlen(filename) + 1);
        char* msg = malloc(msg_size); 

        snprintf(msg, msg_size, msg_format, filename);
        
        String str = {0};
        String_set(&str, msg);
        free(msg);

        editor_push_message(state, &str);
        String_set(&(state->current_file), filename);
    }

    String_clear(&filepath);

    return return_code;
}


int editor_open_file(ProgramState* state, const char* filename)
{
    int return_code = FILEIO_PATH_WAS_INVALID;
    if (filename == NULL)
    {
        printf("No file selected to open.\n");
        return return_code;
    }

    char* msg_format = NULL;
    FILE* fp;
    String filepath = {0};

    //Append filename to current directory
    String_insert_string(&filepath, filename, 0);
    String_insert(&filepath, '\\', 0);
    String_insert_string(&filepath, state->current_directory.text, 0);
    fopen_s(&fp, filepath.text, "r");

    if (!fp)
    {
        int attributes = GetFileAttributes(filepath.text);
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            String_set(&(state->current_directory), filepath.text);
            return_code = FILEIO_PATH_WAS_DIRECTORY;
        }
        else
        {
            msg_format = "Couldn't open file '%s'.";
        }
    }
    else
    {
        String_clear(&(state->text.text));
        state->text.cursor_index = 0;
        state->camera_x = 0;
        state->camera_y = 0;
        while (!feof(fp))
        {
            char c = fgetc(fp);
            if (c <= 0) //a weird character appears at the end of every .txt file. not sure why
            {
                continue;
            }
            String_push(&(state->text.text), c);
            if (c == '\n')
            {
                continue;
            }
        }
        fclose(fp);

        msg_format = "Opened file '%s'.";
        return_code = FILEIO_PATH_WAS_FILE;
    }


    if (msg_format)
    {
        size_t msg_size = sizeof(char) * (strlen(msg_format) + strlen(filename) + 1);
        char* msg = malloc(msg_size); 

        snprintf(msg, msg_size, msg_format, filename);
        
        String str = {0};
        String_set(&str, msg);
        free(msg);

        editor_push_message(state, &str);
        String_set(&(state->current_file), filename);
    }

    String_clear(&filepath);

    return return_code;
}

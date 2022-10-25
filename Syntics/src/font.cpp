#include "font.h"
#include "file_reading.h"
#include "logging.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RESET(thing, bytes) memset(thing, 0, bytes)
#define MAX_WORD_LEN 30

#define READ_HEADER 0
#define READ_CHARS 1
#define READ_CHAR 2

#define READ_ID 0
#define READ_X 1
#define READ_Y 2
#define READ_WIDTH 3
#define READ_HEIGHT 4
#define READ_X_OFFSET 5
#define READ_Y_OFFSET 6
#define READ_X_ADVANCE 7

namespace synt {

Character::Character()
    : id(0), x(0), y(0), width(0), height(0), x_offset(0), y_offset(0),
      x_advance(0)
{
}

int32& Character::operator[](int i)
{
    switch (i)
    {
        case READ_ID:
        {
            return id;
        }
        case READ_X:
        {
            return x;
        }
        case READ_Y:
        {
            return y;
        }
        case READ_WIDTH:
        {
            return width;
        }
        case READ_HEIGHT:
        {
            return height;
        }
        case READ_X_OFFSET:
        {
            return x_offset;
        }
        case READ_Y_OFFSET:
        {
            return y_offset;
        }
        case READ_X_ADVANCE:
        {
            return x_advance;
        }
        default:
        {
            ERROR("Index out of bounds!");
        }
    }
    ERROR("Index out of bounds!");
    return id;
}

Font::Font() : width_atlas(0), height_atlas(0) {}

#define get_word(file, index, buffer, new_line)                                \
    ({                                                                         \
        if (!_get_word(file, index, buffer, new_line))                         \
        {                                                                      \
            end_of_file = true;                                                \
            break;                                                             \
        }                                                                      \
    })

static bool _get_word(File_Attrib* file, uint32_t* index, char* buffer,
                      bool* new_line)
{
    while (file->buffer[*index] == ' ' || file->buffer[*index] == '=')
    {
        if (++(*index) >= file->size) return false;
    }
    uint32_t len = 0;
    while (file->buffer[*index] != ' ' && file->buffer[*index] != '=' &&
           file->buffer[*index] != '\n')
    {
        if (len >= MAX_WORD_LEN) ERROR("WORD TO FLIPPING LONG");
        buffer[len++] = file->buffer[*index];
        if (++(*index) >= file->size) return false;
    }
    buffer[len] = '\0';

    if (file->buffer[*index] == '\n')
        *new_line = true;
    else
        *new_line = false;

    return true;
}

Font load_font_file(const char* file_path)
{

    File_Attrib file   = read_file(NULL, file_path, "r");
    uint32_t value_len = 0;
    char word[MAX_WORD_LEN];
    RESET(word, sizeof(word));

    Font out;

    uint32_t num_chars = 0;
    uint32_t mode      = 0;

    bool header_read = false, new_line = false, end_of_file = false;
    for (uint32_t i = 0; i < file.size; i++)
    {
        while (file.buffer[i] != '\n')
        {
            get_word(&file, &i, word, &new_line);
            if (!header_read)
            {
                if (!strcmp(word, "chars"))
                {
                    get_word(&file, &i, word, &new_line);
                    mode = READ_CHARS;
                }
                else if (!strcmp(word, "char"))
                {
                    header_read = true;
                    mode        = READ_CHAR;
                }
                else if (!strcmp(word, "scaleW"))
                {
                    get_word(&file, &i, word, &new_line);
                    out.width_atlas = atoi(word);
                }
                else if (!strcmp(word, "scaleH"))
                {
                    get_word(&file, &i, word, &new_line);
                    out.height_atlas = atoi(word);
                }
            }
            if (i >= file.size - 1)
            {
                end_of_file = true;
                break;
            }
            switch (mode)
            {
                case READ_HEADER:
                {
                    break;
                }
                case READ_CHARS:
                {
                    if (!strcmp(word, "count"))
                    {
                        get_word(&file, &i, word, &new_line);
                        num_chars = atoi(word);
                        out.characters.reserve(num_chars);
                        break;
                    }
                }
                case READ_CHAR:
                {
                    if (strcmp(word, "char"))
                    {
                        end_of_file = true;
                        break;
                    }
                    int counter = 0;
                    int id      = 0;
                    while (!new_line)
                    {
                        get_word(&file, &i, word, &new_line);
                        if (counter <= READ_X_ADVANCE)
                        {
                            get_word(&file, &i, word, &new_line);
                            if (counter == READ_ID) id = atoi(word);
                            out.characters[id][counter++] = atoi(word);
                        }
                    }
                    if (out.characters.size() == num_chars)
                    {
                        end_of_file = true;
                        break;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (end_of_file) break;
            if (new_line) continue;
            i++;
        }
        if (end_of_file) break;
    }
    return out;
}

uint32_t text(Font font, const char* text) {}

} // namespace synt

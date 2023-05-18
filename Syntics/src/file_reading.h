#pragma once

#include "defines.h"

typedef struct File_Attrib
{
    unsigned char* buffer;
    u32 size; // PADDING: 4 bytes padding
} File_Attrib;

void read_file_offset_arr(File_Attrib* file_attrib, Region_Alloc* region,
                          const char* file_path, const char* operation);

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);

void write_to_file(const char* file_path, const char* content);

void write_entire_file(const char* file_path, const char* content, u32 size);

void free_file(File_Attrib* file_attrib);

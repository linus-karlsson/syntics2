#pragma once

#include "defines.h"

typedef struct File_Attrib
{
    unsigned char* buffer;
    u32 size;
    b8 region_based;
} File_Attrib;

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);

void write_to_file(const char* file_path, const char* content);

void write_entire_file(const char* file_path, const char* content);

void free_file(File_Attrib* file_attrib);

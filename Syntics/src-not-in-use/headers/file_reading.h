#pragma once

//#include "defines.h"

typedef struct File_Attrib
{
    unsigned char* buffer;
    u32 current_pos;
    u32 size;
} File_Attrib;

void read_file_offset_arr(File_Attrib* file_attrib, Region_Alloc* region,
                          const char* file_path, const char* operation);

b8 end_of_file(const File_Attrib& file);

u32 read_line(File_Attrib* file, char* line, u32 max_size);

char* read_token(char* buffer,const char* delims);

void read_file(File_Attrib* file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);

void write_to_file(const char* file_path, const char* content);

void write_entire_file(const char* file_path, const char* content, u32 size);

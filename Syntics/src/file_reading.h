#pragma once

#include "defines.h"

typedef struct File_Attrib
{
    File_Attrib();
    ~File_Attrib();
    unsigned char* buffer;
    uint32 size;
    bool region_based;
} File_Attrib;

typedef struct Region_Alloc Region_Alloc;

void read_file(File_Attrib& file_attrib, Region_Alloc* region, const char* file_path,
               const char* operation);


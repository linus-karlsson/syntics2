#pragma once

#include "defines.h"

namespace synt {

typedef struct File_Attrib
{
    File_Attrib();
    ~File_Attrib();
    unsigned char* buffer;
    uint32 size;
    bool region_based;
} File_Attrib;

typedef struct Region_Alloc Region_Alloc;

File_Attrib read_file(Region_Alloc* region, const char* file_path,
                      const char* operation);

} // namespace synt

#pragma once

#include "region_alloc.h"
#include "defines.h"

namespace synt {

typedef struct File_Attrib
{
    Temp_Alloc<char> buffer;
} File_Attrib;

File_Attrib read_file(Region_Alloc* region, const char* file_path,
                      const char* operation);

} // namespace synt

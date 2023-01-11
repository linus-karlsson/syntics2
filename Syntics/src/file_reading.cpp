#include "file_reading.h"
#include "region_alloc.h"
#include <stdlib.h>

namespace synt {

File_Attrib::File_Attrib() : buffer(0), size(0), region_based(0)
{
}
File_Attrib::~File_Attrib()
{
    if (!region_based)
    {
        if (buffer) free(buffer);
    }
}

File_Attrib read_file(Region_Alloc* region, const char* file_path,
                      const char* operation)
{
    FILE* file = fopen(file_path, operation);

    if (file == NULL) SY_ERROR(file_path);

    File_Attrib file_attrib;

    fseek(file, 0, SEEK_END);
    file_attrib.size = (uint32)ftell(file);
    rewind(file);

    if (region)
    {
        file_attrib.buffer = region_mallocT(region, file_attrib.size, unsigned char);
        file_attrib.region_based = true;
    }
    else
    {
        file_attrib.buffer = (unsigned char*)malloc(file_attrib.size);
        file_attrib.region_based = false;
    }

    if (fread(file_attrib.buffer, 1, file_attrib.size, file) != file_attrib.size)
    {
        SY_ERROR(file_path);
    }
    fclose(file);

    return file_attrib;
}

} // namespace synt

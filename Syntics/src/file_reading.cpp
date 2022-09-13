#include "file_reading.h"
#include "logging.h"
#include <stdlib.h>

namespace synt {

File_Attrib read_file(Region_Alloc* region, const char* file_path,
                      const char* operation)
{
    FILE* file = fopen(file_path, operation);

    if (file == NULL) ERROR(file_path);

    File_Attrib file_attrib;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    file_attrib.buffer.init(region, (uint32)size);

    if (fread(file_attrib.buffer.data, 1, size, file) != size)
    {
        ERROR(file_path);
    }

    return file_attrib;
}

} // namespace synt

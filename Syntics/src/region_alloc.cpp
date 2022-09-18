#include "region_alloc.h"
#include <string.h>
#include <stdlib.h>

namespace synt {

const uint32 SAFTY_FLAG = 378294619;

Region_Alloc::Region_Alloc() : buffer(NULL), capacity(0), currentPos(0), _count_check(0)
{
}
Region_Alloc::~Region_Alloc()
{
    if (buffer) free_region(this);
}

bool init_region(Region_Alloc* region, uint32 size)
{
    if (region != NULL && region->buffer == NULL)
    {
        region->buffer = (unsigned char*)calloc(size, sizeof(unsigned char));

        if (region->buffer == NULL) return 0;

        region->capacity           = size;
        region->currentPos         = 0;
        region->types[TEMP_MALLOC] = 0;
        region->types[PERM_MALLOC] = 0;
        region->types[TEMP_ARRAY]  = 0;
        region->types[PERM_ARRAY]  = 0;

        return 1;
    }
    return 0;
}

void* _region_malloc(Region_Alloc* region, uint32 size, Alloc_Type alloc_type)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        assert(size < region->capacity - region->currentPos);

        unsigned char* currentPos = region->buffer + region->currentPos;
        region->currentPos += size;
        region->types[alloc_type] += 1;
        return currentPos;
    }
    else
    {
        init_region(region, 1000000);
        return _region_malloc(region, size, alloc_type);
    }

    return NULL;
}

void _region_pop(Region_Alloc* region, uint32 size, Alloc_Type alloc_type)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        if (size > region->currentPos)
        {
            region->currentPos = 0;
        }
        else if (alloc_type == TEMP_ARRAY || alloc_type == PERM_ARRAY)
        {
            region->currentPos -= (size + sizeof(Array_Head));
        }
        else
        {
            region->currentPos -= size;
        }
        region->types[alloc_type] -= 1;
    }
    else
    {
        ERROR("Region is not initialized: use init_region() at the start of "
              "the program\n");
    }
}

void reset_region(Region_Alloc* region) { region->currentPos = 0; }

void free_region(Region_Alloc* region)
{
    if (region->buffer) free(region->buffer);
    region->buffer = NULL;
}

void print_region(const Region_Alloc& region)
{
    synt_LOG("\nTotal memory: %d\n", (int)(region.capacity));
    synt_LOG("Total memory used: %d\n", (int)(region.currentPos));
    synt_LOG("Total memory left: %d\n", (int)(region.capacity - region.currentPos));

    synt_LOG("\nPERM Malloc allocations: %d\n", (region.types[PERM_MALLOC]));
    synt_LOG("PERM Array allocations: %d\n", (region.types[PERM_ARRAY]));
    synt_LOG("TEMP Malloc allocations: %d\n", (region.types[TEMP_MALLOC]));
    synt_LOG("TEMP Array allocations: %d\n\n", (region.types[TEMP_ARRAY]));
}

void* _dyn_array(Region_Alloc* region, uint32 capacity, uint32 type,
                 Alloc_Type alloc_type, uint32 extra_size)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        const uint32 size = capacity * type;

        assert((size < region->capacity - region->currentPos) && "Not enough memory");

        Array_Head* headPos = (Array_Head*)(region->buffer + region->currentPos);

        *headPos++ = (Array_Head){ capacity, 0, SAFTY_FLAG };

        region->currentPos += (size + sizeof(Array_Head) + extra_size);

        region->types[alloc_type] += 1;

        return (void*)headPos;
    }
    else
    {
        init_region(region, 1000000);
        return _dyn_array(region, capacity, type, alloc_type, 0);
    }

    return NULL;
}
void* _dyn_array_calloc(Region_Alloc* region, uint32 capacity, uint32 type,
                        Alloc_Type alloc_type)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        const uint32 size = capacity * type;

        assert((size < region->capacity - region->currentPos) && "Not enough memory");

        Array_Head* headPos = (Array_Head*)(region->buffer + region->currentPos);

        *headPos++ = (Array_Head){ capacity, 0, SAFTY_FLAG };

        memset(headPos, 0, size);
        region->currentPos += (size + sizeof(Array_Head));
        region->types[alloc_type] += 1;

        return (void*)headPos;
    }
    else
    {
        init_region(region, 1000000);
        return _dyn_array(region, capacity, type, alloc_type, 0);
    }

    return NULL;
}
void* _simple_dyn_array_calloc(Region_Alloc* region, uint32 capacity, uint32 type,
                               Alloc_Type alloc_type)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        const uint32 size = capacity * type;

        assert((size < region->capacity - region->currentPos) && "Not enough memory");

        Simple_Array_Head* headPos =
            (Simple_Array_Head*)(region->buffer + region->currentPos);

        *headPos++ = (Simple_Array_Head){ capacity, 0 };

        memset(headPos, 0, size);
        region->currentPos += (size + sizeof(Simple_Array_Head));
        region->types[alloc_type] += 1;

        return (void*)headPos;
    }
    else
    {
        init_region(region, 1000000);
        return _dyn_array(region, capacity, type, alloc_type, 0);
    }

    return NULL;
}

void* _dyn_array_val(Region_Alloc* region, uint32 numElements, uint32 capacity,
                     uint32 type, Alloc_Type alloc_type, const void* values)
{
    assert(region);
    if (region != NULL && region->buffer != NULL)
    {
        const uint32 size = capacity * type;

        assert((size < region->capacity - region->currentPos) && "Not enough memory");

        Array_Head* headPos = (Array_Head*)(region->buffer + region->currentPos);

        *headPos++ = (Array_Head){ capacity, numElements, SAFTY_FLAG };

        memcpy(headPos, values, size);
        region->currentPos += (size + sizeof(Array_Head));
        region->types[alloc_type] += 1;

        return (void*)headPos;
    }
    else
    {
        init_region(region, 1000000);
        return _dyn_array_val(region, numElements, capacity, type, alloc_type, values);
    }

    return NULL;
}

Array_Head* _check_array(void* array)
{
    Array_Head* checkValue = ((Array_Head*)(((Array_Head*)array) - 1));

    if (checkValue->safetyFlag == SAFTY_FLAG)
    {
        return checkValue;
    }

    ERROR("Not an array with size header\n");

    return NULL;
}

Array_Head* _check_array_push(void* array)
{
    Array_Head* checkValue = (((Array_Head*)array) - 1);

    if (checkValue->safetyFlag == SAFTY_FLAG && checkValue->size < checkValue->capacity)
    {
        return checkValue;
    }

    ERROR("Not an array with size header, or out of capacity");

    return NULL;
}

Array_Head* _dyn_check_array_push(void* array)
{
    Array_Head* checkValue = ((Array_Head*)(((Array_Head*)array) - 1));

    if (checkValue->safetyFlag == SAFTY_FLAG && checkValue->size < checkValue->capacity)
    {
        return checkValue;
    }
    else if (checkValue->size < checkValue->capacity)
    {
    }

    ERROR("Not an array with size header, or out of capacity");

    return NULL;
}

bool _check_array_size(void* array, uint32 index)
{
    Array_Head* checkValue = ((Array_Head*)(((Array_Head*)array) - 1));
    if (checkValue->safetyFlag == SAFTY_FLAG && index < checkValue->size)
    {
        return 1;
    }

    ERROR("Index out of bounds");

    return 0;
}

void _array_clear(void* array, uint32 stride)
{
    Array_Head* head = _check_array(array);
    head->size       = 0;
    memset(array, 0, head->capacity * stride);
}

uint32 size_arr(const void* const array)
{
    Array_Head* checkValue = (((Array_Head*)array) - 1);
    assert((checkValue->safetyFlag == SAFTY_FLAG) && "Not an array with size header!");

    return checkValue->size;
}

uint32 capacity_arr(const void* const array)
{
    Array_Head* checkValue = (((Array_Head*)array) - 1);
    assert((checkValue->safetyFlag == SAFTY_FLAG) && "Not an array with size header!");

    return checkValue->capacity;
}

static uint32 _TEMP_ARRAY_ID = 0;

uint32 _get_id()
{
    if (_TEMP_ARRAY_ID >= 4000000) _TEMP_ARRAY_ID = 0;

    return _TEMP_ARRAY_ID++;
}

} // namespace synt

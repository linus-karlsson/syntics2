#include "region_alloc.h"
#include "logging.h"
#ifdef LINUX
#include <sys/mman.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

static Region_Alloc g_stack = { 0 };

void init_stack(u64 size)
{
    if (g_stack.capacity == 0)
    {
        init_region(&g_stack, size);
    }
}

Region_Alloc* get_stack()
{
    return &g_stack;
}

void reset_stack()
{
    g_stack.currentPos = 0;
}

u64 _stack_begin_scope(void)
{
    return g_stack.currentPos;
}

global u64 g_biggest_stack_size = 0;

#define MAX(val1, val2) ((val1) > (val2) ? (val1) : (val2))

void _stack_end_scope(u64 size_at_start)
{
    g_biggest_stack_size = MAX(g_biggest_stack_size, g_stack.currentPos);
    g_stack.currentPos = size_at_start;
}

Region_Alloc region_alloc()
{
    Region_Alloc res = { 0 };
    return res;
}

b8 init_region(Region_Alloc* region, u64 size)
{
    if (region != NULL && region->buffer == NULL)
    {

#ifdef LINUX
        region->buffer = (unsigned char*)mmap(NULL, size, PROT_READ | PROT_WRITE,
                                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (region->buffer == MAP_FAILED) ERROR("init_region");
#else
#if 1
        region->buffer = (unsigned char*)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT,
                                                      PAGE_READWRITE);
#endif

#if 0
        region->buffer = (unsigned char*)calloc(size, 1);
        if (region->buffer == NULL) SY_ERROR("init_region");
#endif

#endif

        region->capacity = size;
        region->currentPos = 0;
        region->types[TEMP_MALLOC] = 0;
        region->types[PERM_MALLOC] = 0;
        region->types[TEMP_ARRAY] = 0;
        region->types[PERM_ARRAY] = 0;

        return 1;
    }
    return 0;
}

static void* init_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type)
{
    ASSERT(region, "init_malloc region");
    if (region != NULL && region->buffer != NULL)
    {
        ASSERT(size < region->capacity - region->currentPos,
               "init_array not enough memory");

        unsigned char* currentPos = region->buffer + region->currentPos;
        region->currentPos += size;
        region->types[alloc_type] += 1;
        return currentPos;
    }
    else
    {
        init_region(region, MEGABYTE(10));
        return _region_malloc(region, size, alloc_type);
    }
}

void* _region_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type)
{
    return init_malloc(region, size, alloc_type);
}

void* _region_calloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type)
{
    void* res = init_malloc(region, size, alloc_type);
    memset(res, 0, size);
    return res;
}

void _region_pop(Region_Alloc* region, u32 size, Alloc_Type alloc_type)
{
    ASSERT(region, "region pop");
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
        SY_ERROR("Region is not initialized: use init_region() at the start of "
                 "the program\n");
    }
}

void reset_region(Region_Alloc* region)
{
    region->currentPos = 0;
}

#if 1
void free_region(Region_Alloc* region)
{
}
#endif

void print_region(const Region_Alloc* region)
{
#if 0
    printf("\n");
    synt_LOG("%sTotal memory:%s %llu\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             region.capacity);
    synt_LOG("%sTotal memory used:%s %llu\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             region.currentPos);
    synt_LOG("%sTotal memory left:%s %llu\n", ANSI_COLOR_MAGENTA, ANSI_COLOR_RESET,
             region.capacity - region.currentPos);

    printf("\n");
    synt_LOG("%sPERM Malloc allocations:%s %d\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             (region.types[PERM_MALLOC]));
    synt_LOG("%sPERM Array allocations:%s %d\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             (region.types[PERM_ARRAY]));
    synt_LOG("%sTEMP Malloc allocations:%s %d\n", ANSI_COLOR_MAGENTA,
             ANSI_COLOR_RESET, (region.types[TEMP_MALLOC]));
    synt_LOG("%sTEMP Array allocations:%s %d\n\n", ANSI_COLOR_MAGENTA,
             ANSI_COLOR_RESET, (region.types[TEMP_ARRAY]));
#endif
    static int count = 0;
    synt_LOG_Term("\ncount: %d\n", count++);
    synt_LOG_Term("Total memory: %llu\n", region->capacity);
    synt_LOG_Term("Total memory used: %llu\n", region->currentPos);
    synt_LOG_Term("Total memory left: %llu\n", region->capacity - region->currentPos);

    synt_LOG_Term("\nPERM Malloc allocations: %d\n", (region->types[PERM_MALLOC]));
    synt_LOG_Term("PERM Array allocations: %d\n", (region->types[PERM_ARRAY]));

    synt_LOG_Term("Biggest stack: %llu\n", g_biggest_stack_size);
}

static void* init_array(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type, u32 extra_size)
{
    ASSERT(region, "init_array");
    if (region != NULL && region->buffer != NULL)
    {
        const u32 size = capacity * type;

        ASSERT((size < region->capacity - region->currentPos),
               "init array Not enough memory");

        Array_Head* headPos = (Array_Head*)(region->buffer + region->currentPos);
        *headPos = (Array_Head){ capacity, 0 };
        headPos++;

        region->currentPos += (size + sizeof(Array_Head) + extra_size);
        region->types[alloc_type] += 1;

        return (void*)headPos;
    }
    else
    {
        init_region(region, 1000000);
        return init_array(region, capacity, type, alloc_type, 0);
    }
}

void* _dyn_array(Region_Alloc* region, u32 capacity, u32 type, Alloc_Type alloc_type,
                 u32 extra_size)
{
    return init_array(region, capacity, type, alloc_type, extra_size);
}
void* _dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type)
{
    const u32 size = capacity * type;
    Array_Head* headPos = init_array(region, capacity, type, alloc_type, 0);
    memset(headPos, 0, size);
    return (void*)headPos;
}

void* _dyn_array_val(Region_Alloc* region, u32 capacity, u32 type, Alloc_Type alloc_type,
                     const void* values)
{
    const u32 size = capacity * type;
    Array_Head* headPos = init_array(region, capacity, type, alloc_type, 0);
    memcpy(headPos, values, size);
    return (void*)headPos;
}

b8 _check_array_size(void* array)
{
    Array_Head* head = ((Array_Head*)(((Array_Head*)array) - 1));
    if (head->size < head->capacity)
    {
        return true;
    }
    return false;
}
b8 _check_array_size_index(void* array, u32 index)
{
    Array_Head* head = ((Array_Head*)(((Array_Head*)array) - 1));
    if (index < head->size)
    {
        return 1;
    }

    SY_ERROR("Index out of bounds");

    return 0;
}

u32 _check_pop_array_size(void* array)
{
    Array_Head* head = ((Array_Head*)(((Array_Head*)array) - 1));
    if (head->size > 0)
    {
        return --head->size;
    }
    SY_ERROR("Array size to small for popping");
    return 0;
}

void _array_clear(void* array, u32 stride)
{
    Array_Head* head = ((Array_Head*)(((Array_Head*)array) - 1));
    head->size = 0;
    memset(array, 0, head->capacity * stride);
}

u32 size_arr(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);

    return head->size;
}

u32 capacity_arr(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);

    return head->capacity;
}

static u32 _TEMP_ARRAY_ID = 0;

u32 _get_id()
{
    if (_TEMP_ARRAY_ID >= 4000000) _TEMP_ARRAY_ID = 0;

    return _TEMP_ARRAY_ID++;
}


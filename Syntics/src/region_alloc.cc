//#include "region_alloc.h"
//#include "logging.h"
#ifdef LINUX
#include <sys/mman.h>
#else
#if 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include "win32/sy_winalloc.h"
#endif
#endif
#include <string.h>
#include <stdlib.h>

static Region_Alloc g_stack = {};

global u64 CHECK_VALUE = 0xF0524CA8431BEC38;

Array_Head::Array_Head(u32 capacity, u32 size) : capacity(capacity), size(size)
{
#ifdef DEBUG
    m_safety_number = CHECK_VALUE;
#endif
}

#ifdef DEBUG
u64 Array_Head::safety_number()
{
    return m_safety_number;
}
#endif

void init_stack(u32 size)
{
    if (g_stack.capacity == 0)
    {
        init_region(&g_stack, size);
    }
}

Region_Alloc* _get_stack(u32 check_val)
{
    return &g_stack;
}

void reset_stack(void)
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

Region_Alloc region_alloc(void)
{
    Region_Alloc res = {};
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
        region->buffer = (unsigned char*)VirtualAlloc(
            0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
    sy_print("\ncount: %d\n", count++);
    sy_print("Total memory: %llu\n", region->capacity);
    sy_print("Total memory used: %llu\n", region->currentPos);
    sy_print("Total memory left: %llu\n", region->capacity - region->currentPos);

    sy_print("\nPERM Malloc allocations: %d\n", (region->types[PERM_MALLOC]));
    sy_print("PERM Array allocations: %d\n", (region->types[PERM_ARRAY]));

    sy_print("Biggest stack: %llu\n", g_biggest_stack_size);
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

        Array_Head* head_pos = (Array_Head*)(region->buffer + region->currentPos);
        *head_pos = Array_Head(capacity, 0);
        head_pos++;

        region->currentPos += (size + sizeof(Array_Head) + extra_size);
        region->types[alloc_type] += 1;

        return (void*)head_pos;
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
    void* head_pos = init_array(region, capacity, type, alloc_type, 0);
    memset(head_pos, 0, size);
    return head_pos;
}

void* _dyn_array_val(Region_Alloc* region, u32 capacity, u32 type,
                     Alloc_Type alloc_type, const void* values)
{
    const u32 size = capacity * type;
    void* head_pos = init_array(region, capacity, type, alloc_type, 0);
    memcpy(head_pos, values, size);
    return head_pos;
}

Array_Head* _check_array(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->safety_number() == CHECK_VALUE);
    return head;
}

b8 _check_array_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    ASSERT(head->safety_number() == CHECK_VALUE, "Array Do not have a size");
    if (head->size < head->capacity)
    {
        return true;
    }
    return false;
}
u32 _check_array_size_index(void* array, u32 index)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
#ifdef DEBUG
    ASSERT(head->safety_number() == CHECK_VALUE, "Array Do not have a size");
#endif
    if (index < head->capacity)
    {
        return index;
    }

    SY_ERROR("Index out of bounds");
    return 0;
}

u32 _check_pop_array_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
#ifdef DEBUG
    ASSERT(head->safety_number() == CHECK_VALUE, "Array Do not have a size");
#endif
    if (head->size > 0)
    {
        return --head->size;
    }
    SY_ERROR("Array size to small for popping");
    return 0;
}

void _array_clear(void* array, u32 stride)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    head->size = 0;
    memset(array, 0, head->capacity * stride);
}

u32 size_arr(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
#ifdef DEBUG
    ASSERT(head->safety_number() == CHECK_VALUE, "Array Do not have a size");
#endif

    return head->size;
}

u32 capacity_arr(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
#ifdef DEBUG
    ASSERT(head->safety_number() == CHECK_VALUE, "Array Do not have a size");
#endif

    return head->capacity;
}

static u32 _TEMP_ARRAY_ID = 0;

u32 _get_id(void)
{
    if (_TEMP_ARRAY_ID >= 4000000) _TEMP_ARRAY_ID = 0;

    return _TEMP_ARRAY_ID++;
}


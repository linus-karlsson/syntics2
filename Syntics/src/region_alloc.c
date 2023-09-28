#ifndef SY_UNIT_BUILD
#include "region_alloc.h"
#include "logging.h"
#endif

global Region_Alloc REGION_g_stack = { 0 };

global u64 REGION_CHECK_VALUE = 0xF0524CA8431BEC38;

Array_Head array_head_create(u32 capacity, u32 size)
{
    Array_Head out = { 0 };
    out.size = size;
    out.capacity = capacity;
    out._safety_number = REGION_CHECK_VALUE;
    return out;
}


b8 region_init(Region_Alloc* region, u64 size)
{
    region->buffer = (u8*)virtual_allocation(size);
#if 0
        region->buffer = (unsigned char*)calloc(size, 1);
        assert(region->buffer);
#endif

    region->capacity = size;
    region->current_pos = 0;

    return 1;
}

void stack_init(u32 size)
{
    region_init(&REGION_g_stack, size);
}

Region_Alloc* _stack_get(u32 check_val)
{
    return &REGION_g_stack;
}

u64 stack_size(void)
{
    return REGION_g_stack.current_pos;
}

void stack_reset(void)
{
    REGION_g_stack.current_pos = 0;
}

u64 _stack_begin_scope(void)
{
    return REGION_g_stack.current_pos;
}

global u64 g_biggest_stack_size = 0;

#define MAX(val1, val2) ((val1) > (val2) ? (val1) : (val2))

void _stack_end_scope(u64 size_at_start)
{
    g_biggest_stack_size =
        MAX(g_biggest_stack_size, REGION_g_stack.current_pos);
    REGION_g_stack.current_pos = size_at_start;
}

internal u32 alignment_offset_get(u8* current_pos, u32 alignment)
{
    const uintptr_t current_ptr = (uintptr_t)current_pos;
    const u32 mask = alignment - 1;
    u32 result = current_ptr & mask;
    if (result)
    {
        result = alignment - result;
    }
    return result;
}

internal void* malloc_init(Region_Alloc* region, u32 size, u32 alignment)
{
    assert(alignment);
    u32 alignment_offset =
        alignment_offset_get(region->buffer + region->current_pos, alignment);

    assert((size + alignment_offset) <
           (region->capacity - region->current_pos));

    region->current_pos += alignment_offset;

    unsigned char* current_pos = region->buffer + region->current_pos;
    region->current_pos += size;
    return current_pos;
}

void* _region_malloc(Region_Alloc* region, u32 size, u32 alignment)
{
    return malloc_init(region, size, alignment);
}

void* _region_calloc(Region_Alloc* region, u32 size, u32 alignment)
{
    void* res = malloc_init(region, size, alignment);
    memset(res, 0, size);
    return res;
}

void _region_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type)
{
    assert(false);

    if (size > region->current_pos)
    {
        region->current_pos = 0;
    }
    else if (alloc_type == ARRAY)
    {
        region->current_pos -= (size + sizeof(Array_Head));
    }
    else
    {
        region->current_pos -= size;
    }
}

void region_reset(Region_Alloc* region)
{
    region->current_pos = 0;
}

#if 1
void region_free(Region_Alloc* region)
{
    free_allocation(region->buffer, region->capacity);
}
#endif

void region_print(const Region_Alloc* region)
{
    static int count = 0;
    sy_print("\ncount: %d\n", count++);
    sy_print("Total memory: %llu\n", region->capacity);
    sy_print("Total memory used: %llu\n", region->current_pos);
    sy_print("Total memory left: %llu\n",
             region->capacity - region->current_pos);

    sy_print("Biggest stack: %llu\n", g_biggest_stack_size);
}

internal void* array_init(Region_Alloc* region, u32 capacity, u32 type,
                        u32 alignment)
{
    assert(alignment);
    const u32 array_head_size = sizeof(Array_Head);
    assert(region->current_pos + array_head_size < region->capacity);

    region->current_pos += array_head_size;

    u32 alignment_offset =
        alignment_offset_get(region->buffer + region->current_pos, alignment);

    const u32 size = capacity * type;
    assert((size + alignment_offset) <
           (region->capacity - region->current_pos));

    region->current_pos += alignment_offset;

    Array_Head* head_pos =
        (Array_Head*)(region->buffer + (region->current_pos - array_head_size));

    *head_pos = array_head_create(capacity, 0);
    head_pos++;

    region->current_pos += size;

    return (void*)head_pos;
}

void* _region_array(Region_Alloc* region, u32 capacity, u32 type, u32 alignment)
{
    return array_init(region, capacity, type, alignment);
}
void* _region_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                           u32 alignment)
{
    const u32 size = capacity * type;
    void* head_pos = array_init(region, capacity, type, alignment);
    memset(head_pos, 0, size);
    return head_pos;
}

void* _region_array_val(Region_Alloc* region, u32 capacity, u32 type,
                        u32 alignment, const void* values)
{
    const u32 size = capacity * type;
    void* head_pos = array_init(region, capacity, type, alignment);
    memcpy(head_pos, values, size);
    return head_pos;
}

Array_Head* _array_check(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE);
    return head;
}

b8 _array_check_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE &&
           "Array Do not have a size");
    if (head->size < head->capacity)
    {
        return true;
    }
    return false;
}

u32 _array_check_size_index(void* array, u32 index)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE &&
           "Array Do not have a size");
    if (index < head->capacity)
    {
        return index;
    }

    SY_ERROR("Index out of bounds");
    return 0;
}

u32 _array_check_pop_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE &&
           "Array Do not have a size");
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

u32 array_size(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE &&
           "Array Do not have a size");

    return head->size;
}

u32 array_capacity(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE &&
           "Array Do not have a size");

    return head->capacity;
}

char* path_extend(Region_Alloc* region, const char* trailing_path,
                  u32 trailing_path_len)
{
    char* result =
        region_array(region, WORKING_DIR_LEN + trailing_path_len + 1, char);
    memcpy(result, WORKING_DIR, WORKING_DIR_LEN);
    memcpy(result + WORKING_DIR_LEN, trailing_path, trailing_path_len);
    array_val(result, WORKING_DIR_LEN + trailing_path_len) = '\0';
    return result;
}

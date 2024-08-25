#ifndef SY_UNIT_BUILD
#include "region_alloc.h"
#include "logging.h"
#include "syntics_platform.h"
#endif

global Region_Alloc REGION_g_stack = { 0 };
global const u64 REGION_CHECK_VALUE = 0xF0524CA8431BEC38;

global char* WORKING_DIR = NULL;
global u32 WORKING_DIR_LEN = 0;

Array_Head region_array_head_create(u32 capacity, u32 size)
{
    Array_Head out = { 0 };
    out.size = size;
    out.capacity = capacity;
    out.safety_number_ = REGION_CHECK_VALUE;
    return out;
}

b8 region_init(Region_Alloc* region, u64 size)
{
    region->buffer = (u8*)platform_virtual_allocation(size);
#if 0
        region->buffer = (unsigned char*)calloc(size, 1);
        assert(region->buffer);
#endif

    region->capacity = size;
    region->current_pos = 0;
    region->mutex = platform_semaphore_create(1, 100);

    return 1;
}

void region_stack_init(u32 size)
{
    region_init(&REGION_g_stack, size);
}

Region_Alloc* region_i_stack_get(u32 check_val)
{
    return &REGION_g_stack;
}

u64 region_stack_size(void)
{
    return REGION_g_stack.current_pos;
}

void region_stack_reset(void)
{
    REGION_g_stack.current_pos = 0;
}

u64 region_i_stack_begin_scope(void)
{
    return REGION_g_stack.current_pos;
}

global u64 g_biggest_stack_size = 0;

#define MAX(val1, val2) ((val1) > (val2) ? (val1) : (val2))

void region_i_stack_end_scope(u64 size_at_start)
{
    g_biggest_stack_size = MAX(g_biggest_stack_size, REGION_g_stack.current_pos);
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
    u32 alignment_offset = alignment_offset_get(region->buffer + region->current_pos, alignment);

    assert((size + alignment_offset) < (region->capacity - region->current_pos));

    region->current_pos += alignment_offset;

    unsigned char* current_pos = region->buffer + region->current_pos;
    region->current_pos += size;
    return current_pos;
}

void* region_i_malloc(Region_Alloc* region, u32 size, u32 alignment)
{
    platform_semaphore_wait_and_decrement(&region->mutex);
    void* result = malloc_init(region, size, alignment);
    platform_semaphore_increment(&region->mutex);
    return result;
}

void* region_i_calloc(Region_Alloc* region, u32 size, u32 alignment)
{
    platform_semaphore_wait_and_decrement(&region->mutex);
    void* res = malloc_init(region, size, alignment);
    memset(res, 0, size);
    platform_semaphore_increment(&region->mutex);
    return res;
}

void region_i_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type)
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
    platform_semaphore_wait_and_decrement(&region->mutex);
    region->current_pos = 0;
    platform_semaphore_increment(&region->mutex);
}

#if 1
void region_free(Region_Alloc* region)
{
    platform_free_allocation(region->buffer, region->capacity);
}
#endif

void region_print(const Region_Alloc* region)
{
    static int count = 0;
    sy_print("\ncount: %d\n", count++);
    sy_print("Total memory: %llu\n", region->capacity);
    sy_print("Total memory used: %llu\n", region->current_pos);
    sy_print("Total memory left: %llu\n", region->capacity - region->current_pos);

    sy_print("Biggest stack: %llu\n", g_biggest_stack_size);
}

internal void* array_init(Region_Alloc* region, u32 capacity, u32 type, u32 alignment)
{
    assert(alignment);
    const u32 region_array_head_size = sizeof(Array_Head);
    assert(region->current_pos + region_array_head_size < region->capacity);

    region->current_pos += region_array_head_size;

    u32 alignment_offset = alignment_offset_get(region->buffer + region->current_pos, alignment);

    const u32 size = capacity * type;
    assert((size + alignment_offset) < (region->capacity - region->current_pos));

    region->current_pos += alignment_offset;

    Array_Head* head_pos =
        (Array_Head*)(region->buffer + (region->current_pos - region_array_head_size));

    *head_pos = region_array_head_create(capacity, 0);
    head_pos++;

    region->current_pos += size;

    return (void*)head_pos;
}

void* region_i_array(Region_Alloc* region, u32 capacity, u32 type, u32 alignment)
{
    platform_semaphore_wait_and_decrement(&region->mutex);
    void* result = array_init(region, capacity, type, alignment);
    platform_semaphore_increment(&region->mutex);
    return result;
}
void* region_i_array_calloc(Region_Alloc* region, u32 capacity, u32 type, u32 alignment)
{
    platform_semaphore_wait_and_decrement(&region->mutex);
    const u32 size = capacity * type;
    void* result = array_init(region, capacity, type, alignment);
    memset(result, 0, size);
    platform_semaphore_increment(&region->mutex);
    return result;
}

void* region_i_array_val(Region_Alloc* region, u32 capacity, u32 type, u32 alignment,
                         const void* values)
{
    platform_semaphore_wait_and_decrement(&region->mutex);
    const u32 size = capacity * type;
    void* result = array_init(region, capacity, type, alignment);
    memcpy(result, values, size);
    platform_semaphore_increment(&region->mutex);
    return result;
}

Array_Head* region_i_array_check(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE);
    return head;
}

b8 region_i_array_check_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE && "Array Do not have a size");
    if (head->size < head->capacity)
    {
        return true;
    }
    return false;
}

u32 region_i_array_check_size_index(void* array, u32 index)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE && "Array Do not have a size");
    if (index < head->capacity)
    {
        return index;
    }

    SY_ERROR("Index out of bounds");
    return 0;
}

u32 region_i_array_check_pop_size(void* array)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE && "Array Do not have a size");
    if (head->size > 0)
    {
        return --head->size;
    }
    SY_ERROR("Array size to small for popping");
    return 0;
}

void region_i_array_clear(void* array, u32 stride)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    head->size = 0;
    memset(array, 0, head->capacity * stride);
}

u32 region_array_size(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE && "Array Do not have a size");

    return head->size;
}

u32 region_array_capacity(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
    assert(head->safety_number_ == REGION_CHECK_VALUE && "Array Do not have a size");

    return head->capacity;
}

void find_working_dir(Region_Alloc* region)
{
    char file[MAX_PATH];
    u32 len = platform_get_executable_directory(file, MAX_PATH);
    char* token = NULL;
    i32 steps = -1;
    for (; len > 0; len--)
    {
        steps++;
        if (file[len - 1] == '\\' || file[len - 1] == '/')
        {
            token = file + len;
            char temp = token[steps];
            token[steps] = '\0';
            if (!strcmp(token, "syntics2"))
            {
                token[steps] = temp;
                len += steps + 1;
                break;
            }
            token[steps] = temp;
            steps = -1;
        }
    }
    assert(len > 1);
    WORKING_DIR = region_array(region, len + 1, char);
    memcpy(WORKING_DIR, file, len);
    region_array_value(WORKING_DIR, len) = '\0';
    WORKING_DIR_LEN = len;
}

char* path_extend(Region_Alloc* region, const char* trailing_path, u32 trailing_path_len)
{
    char* result = region_array(region, WORKING_DIR_LEN + trailing_path_len + 1, char);
    memcpy(result, WORKING_DIR, WORKING_DIR_LEN);
    memcpy(result + WORKING_DIR_LEN, trailing_path, trailing_path_len);
    region_array_value(result, WORKING_DIR_LEN + trailing_path_len) = '\0';
    return result;
}

u32 path_extend2(const char* trailing_path, char* path) 
{
    const u32 trailing_path_len = (u32)strlen(trailing_path);
    memcpy(path, WORKING_DIR, WORKING_DIR_LEN);
    memcpy(path + WORKING_DIR_LEN, trailing_path, trailing_path_len);
    path[WORKING_DIR_LEN + trailing_path_len] = '\0';
    return WORKING_DIR_LEN + trailing_path_len;
}

#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

#define region_stack_malloc(num_elements, type)     (type*)region_i_malloc(region_stack_get(), (num_elements) * sizeof(type), _Alignof(type))
#define region_stack_array(capacity, type)          (type*)region_i_array(region_stack_get(), capacity, sizeof(type), _Alignof(type))
#define region_stack_calloc(num_elements, type)     (type*)region_i_calloc(region_stack_get(), (num_elements) * sizeof(type), _Alignof(type))
#define region_stack_array0(capacity, type)         (type*)region_i_array_calloc(region_stack_get(), capacity, sizeof(type), _Alignof(type))
#define region_stack_pop_malloc(num_elements, type) region_i_pop(region_stack_get(), (num_elements) * sizeof(type))
#define region_stack_pop_array(num_elements, type)  region_i_pop(region_stack_get(), (num_elements) * sizeof(type))
#define region_stack_begin_scope(stack_name)        u64 stack_name = region_i_stack_begin_scope()
#define region_stack_end_scope(stack_name)          region_i_stack_end_scope(stack_name);
#define region_stack_get()                          region_i_stack_get(0)

#define region_malloc(region, num_elements, type)   (type*)region_i_malloc(region, (u32)((num_elements) * sizeof(type)), _Alignof(type))
#define region_malloc_struct(region, type)          (type*)region_i_malloc(region, (u32)(sizeof(type)), _Alignof(type))
#define region_calloc(region, num_elements, type)   (type*)region_i_calloc(region, (u32)((num_elements) * sizeof(type)), _Alignof(type))
#define region_calloc_struct(region, type)          (type*)region_i_calloc(region, (u32)sizeof(type), _Alignof(type))
#define region_pop(region, num_elements, type)      region_i_pop(region, (num_elements) * sizeof(type))

#define region_array(region, capacity, type)        (type*)region_i_array(region, capacity, (u32)sizeof(type), _Alignof(type))
#define region_array_calloc(region, capacity, type) (type*)region_i_array_calloc(region, capacity, (u32)sizeof(type), _Alignof(type))
#define region_array_copy(region, values, capacity, type) (type*)region_i_array_val(region, capacity, (u32)sizeof(type), _Alignof(type), values);
#define region_array_head(array)                    region_i_array_check(array)
#define region_array_back(array)                    ((array) + (region_array_head(array)->size - 1))
#define region_array_reset(array)                   (region_array_head(array)->size = 0)
#define region_array_clear(array, type)             region_i_array_clear(array, sizeof(type))
#define region_array_push(array, value)                                \
    do                                                                         \
    {                                                                          \
        Array_Head* HEAD_INTERNAL_VAL = (((Array_Head*)(array)) - 1);          \
        assert(HEAD_INTERNAL_VAL->size < HEAD_INTERNAL_VAL->capacity &&        \
               ("Array out of size!"__FILE__));                                \
        (array)[HEAD_INTERNAL_VAL->size++] = (value);                          \
    } while (0)
#define region_array_pop(array)                     (array)[region_i_array_check_pop_size((array))]
#define region_array_value_ptr(array, index)        ((array) + region_i_array_check_size_index((array), (index)))
#define region_array_value(array, index)            (*(region_array_value_ptr(array, index)))
#define region_array_value2(array, index, jndex)    region_array_value(region_array_value(array, index), jndex)

#define path_extend_d0(region, path)                path_extend(region, path, (u32)strlen(path))
#define path_extend_d1(path)                        path_extend(region_stack_get(), path, (u32)strlen(path))

typedef enum Allocation_Type
{
    MALLOC,
    ARRAY
} Allocation_Type;

typedef struct Region_Alloc
{
    Semaphore mutex;
    u8* buffer;
    u64 current_pos;
    u64 capacity;
} Region_Alloc;

typedef struct Array_Head
{
    u32 size;
    u32 capacity;
    u64 safety_number_;
} Array_Head;

void  region_stack_init(u32 size);
u64   region_stack_size(void);
void  region_stack_reset(void);
Region_Alloc* 
      region_i_stack_get(u32 check_val);
u64   region_i_stack_begin_scope(void);
void  region_i_stack_end_scope(u64 size_at_start);

b8    region_init(Region_Alloc* region, u64 size);
void* region_i_malloc(Region_Alloc* region, u32 size, u32 alignment);
void* region_i_calloc(Region_Alloc* region, u32 size, u32 alignment);

void* region_i_array(Region_Alloc* region, u32 capacity, u32 type, u32 alignment);
void* region_i_array_calloc(Region_Alloc* region, u32 capacity, u32 type, u32 alignment);
void* region_i_array_val(Region_Alloc* region, u32 capacity, u32 type, u32 alignment, const void* values);

Array_Head* 
      region_i_array_check(void* array);
b8    region_i_array_check_size(void* array);
u32   region_i_array_check_size_index(void* array, u32 index);
u32   region_i_array_check_pop_size(void* array);
void  region_i_array_clear(void* array, u32 stride);
u32   region_array_size(const void* const array);
u32   region_array_capacity(const void* const array);

void  region_i_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type);
void  region_reset(Region_Alloc* region);
void  region_free(Region_Alloc* region);
void  region_print(const Region_Alloc* region);

void  find_working_dir(Region_Alloc* region);
char* path_extend(Region_Alloc* region, const char* trailing_path, u32 trailing_path_len);
u32 path_extend2(const char* trailing_path, char* path);

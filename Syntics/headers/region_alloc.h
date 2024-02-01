#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

#define syntics_region_stack_malloc(num_elements, type)     (type*)i_region_malloc(stack_get(), (num_elements) * sizeof(type), _Alignof(type))
#define syntics_region_stack_array(capacity, type)          (type*)i_region_array(stack_get(), capacity, sizeof(type), _Alignof(type))
#define syntics_region_stack_calloc(num_elements, type)     (type*)i_region_calloc(stack_get(), (num_elements) * sizeof(type), _Alignof(type))
#define syntics_region_stack_array0(capacity, type)         (type*)i_region_array_calloc(stack_get(), capacity, sizeof(type), _Alignof(type))
#define syntics_region_stack_pop_malloc(num_elements, type) syntics_region_i_pop(stack_get(), (num_elements) * sizeof(type))
#define syntics_region_stack_pop_array(num_elements, type)  syntics_region_i_pop(stack_get(), (num_elements) * sizeof(type))
#define syntics_region_stack_begin_scope(stack_name)      u64 stack_name = syntics_region_i_stack_begin_scope()
#define syntics_region_stack_end_scope(stack_name)                         syntics_region_i_stack_end_scope(stack_name);
#define syntics_region_stack_get()                                         syntics_region_i_stack_get(0)

#define syntics_region_malloc(region, num_elements, type)   (type*)i_region_malloc(region, (u32)((num_elements) * sizeof(type)), _Alignof(type))
#define syntics_region_malloc_struct(region, type)          (type*)i_region_malloc(region, (u32)(sizeof(type)), _Alignof(type))
#define syntics_region_calloc(region, num_elements, type)   (type*)i_region_calloc(region, (u32)((num_elements) * sizeof(type)), _Alignof(type))
#define syntics_region_calloc_struct(region, type)          (type*)i_region_calloc(region, (u32)sizeof(type), _Alignof(type))
#define syntics_region_pop(region, num_elements, type)      syntics_region_i_pop(region, (num_elements) * sizeof(type))

#define syntics_region_array(region, capacity, type)        (type*)i_region_array(region, capacity, (u32)sizeof(type), _Alignof(type))
#define syntics_region_array_calloc(region, capacity, type) (type*)i_region_array_calloc(region, capacity, (u32)sizeof(type), _Alignof(type))
#define syntics_region_array_copy(region, values, capacity, type) (type*)i_region_array_val(region, capacity, (u32)sizeof(type), _Alignof(type), values);
#define syntics_region_array_head(array)                    syntics_region_i_array_check(array)
#define syntics_region_array_back(array)                    ((array) + (region_array_head(array)->size - 1))
#define syntics_region_array_reset(array)                   (region_array_head(array)->size = 0)
#define syntics_region_array_clear(array, type)             syntics_region_i_array_clear(array, sizeof(type))
#define syntics_region_array_push(array, value)                                \
    do                                                                         \
    {                                                                          \
        Array_Head* HEAD_INTERNAL_VAL = (((Array_Head*)(array)) - 1);          \
        assert(HEAD_INTERNAL_VAL->size < HEAD_INTERNAL_VAL->capacity &&        \
               ("Array out of size!"__FILE__));                                \
        (array)[HEAD_INTERNAL_VAL->size++] = (value);                          \
    } while (0)
#define syntics_region_array_pop(array)                     (array)[i_array_check_pop_size((array))]
#define syntics_region_array_value_ptr(array, index)        ((array) + syntics_region_i_array_check_size_index((array), (index)))
#define syntics_region_array_value(array, index)            (*(region_array_value_ptr(array, index)))
#define syntics_region_array_value2(array, index, jndex)    syntics_region_array_value(region_array_value(array, index), jndex)

#define path_extend_d0(region, path)                        syntics_path_extend(region, path, (u32)strlen(path))
#define path_extend_d1(path)                                syntics_path_extend(stack_get(), path, (u32)strlen(path))

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

void  syntics_region_stack_init(u32 size);
u64   syntics_region_stack_size(void);
void  syntics_region_stack_reset(void);
Region_Alloc* 
      syntics_region_i_stack_get(u32 check_val);
u64   syntics_region_i_stack_begin_scope(void);
void  syntics_region_i_stack_end_scope(u64 size_at_start);

b8    syntics_region_init(Region_Alloc* region, u64 size);
void* syntics_region_i_malloc(Region_Alloc* region, u32 size, u32 alignment);
void* syntics_region_i_calloc(Region_Alloc* region, u32 size, u32 alignment);

void* syntics_region_i_array(Region_Alloc* region, u32 capacity, u32 type, u32 alignment);
void* syntics_region_i_array_calloc(Region_Alloc* region, u32 capacity, u32 type, u32 alignment);
void* syntics_region_i_array_val(Region_Alloc* region, u32 capacity, u32 type, u32 alignment, const void* values);

Array_Head* 
      syntics_region_i_array_check(void* array);
b8    syntics_region_i_array_check_size(void* array);
u32   syntics_region_i_array_check_size_index(void* array, u32 index);
u32   syntics_region_i_array_check_pop_size(void* array);
void  syntics_region_i_array_clear(void* array, u32 stride);
u32   syntics_region_array_size(const void* const array);
u32   syntics_region_array_capacity(const void* const array);

void  syntics_region_i_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type);
void  syntics_region_reset(Region_Alloc* region);
void  syntics_region_free(Region_Alloc* region);
void  syntics_region_print(const Region_Alloc* region);

void  syntics_find_working_dir(Region_Alloc* region);
char* syntics_path_extend(Region_Alloc* region, const char* trailing_path, u32 trailing_path_len);

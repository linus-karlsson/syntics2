#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

#define stack_malloc(num_elements, type)                                       \
    (type*)_region_malloc(stack_get(), (num_elements) * sizeof(type),            \
                          _Alignof(type))

#define stack_array(capacity, type)                                            \
    (type*)_region_array(stack_get(), capacity, sizeof(type), _Alignof(type))

#define stack_calloc(num_elements, type)                                       \
    (type*)_region_calloc(stack_get(), (num_elements) * sizeof(type),            \
                          _Alignof(type))

#define stack_array0(capacity, type)                                           \
    (type*)_region_array_calloc(stack_get(), capacity, sizeof(type),           \
                                _Alignof(type))

#define stack_pop_malloc(num_elements, type)                                   \
    _region_pop(stack_get(), (num_elements) * sizeof(type))

#define stack_pop_array(num_elements, type)                                    \
    _region_pop(stack_get(), (num_elements) * sizeof(type))

#define region_malloc(region, num_elements, type)                              \
    (type*)_region_malloc(region, (u32)((num_elements) * sizeof(type)),          \
                          _Alignof(type))

#define region_malloc_struct(region, type)                                     \
    (type*)_region_malloc(region, (u32)(sizeof(type)), _Alignof(type))

#define region_calloc(region, num_elements, type)                              \
    (type*)_region_calloc(region, (u32)((num_elements) * sizeof(type)),          \
                          _Alignof(type))

#define region_calloc_struct(region, type)                                     \
    (type*)_region_calloc(region, (u32)sizeof(type), _Alignof(type))

#define region_array(region, capacity, type)                                   \
    (type*)_region_array(region, capacity, (u32)sizeof(type), _Alignof(type))

#define region_array_calloc(region, capacity, type)                            \
    (type*)_region_array_calloc(region, capacity, (u32)sizeof(type),           \
                                _Alignof(type))

#define region_array_copy(region, values, capacity, type)                      \
    (type*)_region_array_val(region, capacity, (u32)sizeof(type),              \
                             _Alignof(type), values);

#define region_pop(region, num_elements, type)                                 \
    _region_pop(region, (num_elements) * sizeof(type))

#define region_array_head(array) _array_check(array)

#define region_array_back(array)                                               \
    ((array) + (region_array_head(array)->size - 1))

#define region_array_reset(array) (region_array_head(array)->size = 0)

#define region_array_clear(array, type) _array_clear(array, sizeof(type))

#define region_array_push(array, value)                                        \
    do                                                                         \
    {                                                                          \
        Array_Head* HEAD_INTERNAL_VAL = (((Array_Head*)(array)) - 1);          \
        assert(HEAD_INTERNAL_VAL->size < HEAD_INTERNAL_VAL->capacity &&        \
               ("Array out of size!"__FILE__));                                \
        (array)[HEAD_INTERNAL_VAL->size++] = (value);                          \
    } while (0)

#define region_array_pop(array) (array)[_array_check_pop_size((array))]

#define region_array_value_ptr(array, index)                                   \
    ((array) + _array_check_size_index((array), (index)))

#define region_array_value(array, index) (*(region_array_value_ptr(array, index)))
#define region_array_value2(array, index, jndex)                                        \
    region_array_value(region_array_value(array, index), jndex)

#if 0
#ifdef DEBUG
#define val(array, index) (*(region_array_value_ptr(array, index)))
#else
#define val(array, index) (array)[index]
#endif
#endif

#define stack_begin_scope(stack_name) u64 stack_name = _stack_begin_scope()
#define stack_end_scope(stack_name) _stack_end_scope(stack_name);
#define stack_begin_defer_end_scope()                                          \
    u64 THIS_IS_NOT28930167 = _stack_begin_scope();                            \
    defer({ stack_end_scope(THIS_IS_NOT28930167); })

#define stack_get() _stack_get(0)

#define defer(func)                                                            \
    goto skip;                                                                 \
    ret:                                                                       \
    func;                                                                      \
    goto ret3;                                                                 \
    skip:

#define Return                                                                 \
    goto ret;                                                                  \
    ret3:                                                                      \
    return

#define scope(name, func)                                                      \
    {                                                                          \
        stack_begin_scope(name);                                               \
        func stack_end_scope(name);                                            \
    }

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
    u32 capacity;
    u32 size;
    u64 _safety_number;
} Array_Head;

b8 region_init(Region_Alloc* region, u64 size);

void stack_init(u32 size);
Region_Alloc* _stack_get(u32 check_val);
u64 stack_size(void);
void stack_reset(void);
u64 _stack_begin_scope(void);
void _stack_end_scope(u64 size_at_start);

void* _region_malloc(Region_Alloc* region, u32 size, u32 alignment);
void* _region_calloc(Region_Alloc* region, u32 size, u32 alignment);

void* _region_array(Region_Alloc* region, u32 capacity, u32 type,
                    u32 alignment);
void* _region_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                           u32 alignment);
void* _region_array_val(Region_Alloc* region, u32 capacity, u32 type,
                        u32 alignment, const void* values);
Array_Head* _array_check(void* array);
b8 _array_check_size(void* array);
u32 _array_check_size_index(void* array, u32 index);
u32 _array_check_pop_size(void* array);
void _array_clear(void* array, u32 stride);
u32 array_size(const void* const array);
u32 array_capacity(const void* const array);

void find_working_dir(Region_Alloc* region);
#define path_extend_d0(region, path)                                           \
    path_extend(region, path, (u32)strlen(path))
#define path_extend_d1(path) path_extend(stack_get(), path, (u32)strlen(path))
char* path_extend(Region_Alloc* region, const char* trailing_path,
                  u32 trailing_path_len);

void _region_pop(Region_Alloc* region, u32 size, Allocation_Type alloc_type);
void region_reset(Region_Alloc* region);
void region_free(Region_Alloc* region);
void region_print(const Region_Alloc* region);

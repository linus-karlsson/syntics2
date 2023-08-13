#pragma once

#define stack_malloc(num_elements, type)                                            \
    (type*)_region_malloc(stack_get(), num_elements * sizeof(type), _Alignof(type))

#define stack_array(capacity, type)                                                 \
    (type*)_region_array(stack_get(), capacity, sizeof(type), _Alignof(type))

#define stack_calloc(num_elements, type)                                            \
    (type*)_region_calloc(stack_get(), num_elements * sizeof(type), _Alignof(type))

#define stack_array0(capacity, type)                                                \
    (type*)_region_array_calloc(stack_get(), capacity, sizeof(type), _Alignof(type))

#define stack_pop_malloc(num_elements, type)                                        \
    _region_pop(stack_get(), num_elements * sizeof(type))

#define stack_pop_array(num_elements, type)                                         \
    _region_pop(stack_get(), num_elements * sizeof(type))

#define region_malloc(region, num_elements, type)                                   \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), _Alignof(type))

#define region_malloc_struct(region, type)                                          \
    (type*)_region_malloc(region, (u32)(1 * sizeof(type)), _Alignof(type))

#define region_calloc(region, num_elements, type)                                   \
    (type*)_region_calloc(region, (u32)(num_elements * sizeof(type)), _Alignof(type))

#define region_calloc_struct(region, type)                                          \
    (type*)_region_calloc(region, (u32)sizeof(type), _Alignof(type))

#define region_array(region, capacity, type)                                        \
    (type*)_region_array(region, capacity, (u32)sizeof(type), _Alignof(type))

#define region_array_calloc(region, capacity, type)                                 \
    (type*)_region_array_calloc(region, capacity, (u32)sizeof(type), _Alignof(type))

#define region_array_copy(region, values, capacity, type)                           \
    (type*)_region_array_val(region, capacity, (u32)sizeof(type), _Alignof(type),   \
                             values);

#define region_pop(region, num_elements, type)                                      \
    _region_pop(region, num_elements * sizeof(type))

#define array_head(array) _array_check(array)

#define array_back(array) ((array) + (array_head(array)->size - 1))

#define array_reset(array) array_head(array)->size = 0

#define array_clear(array, type) _array_clear(array, sizeof(type))

#define array_push(array, value)                                                    \
    do                                                                              \
    {                                                                               \
        Array_Head* HEAD_INTERNAL_VAL = (((Array_Head*)(array)) - 1);               \
        if (HEAD_INTERNAL_VAL &&                                                    \
            HEAD_INTERNAL_VAL->size < HEAD_INTERNAL_VAL->capacity)                  \
            (array)[HEAD_INTERNAL_VAL->size++] = (value);                           \
        else                                                                        \
            assert(!"Array out of size!");                                          \
    } while (0)

#define array_pop(array) (array)[_array_check_pop_size((array))]

#define array_val_ptr(array, index)                                                 \
    ((array) + _array_check_size_index((array), (index)))

#define array_val(array, index) (*(array_val_ptr(array, index)))
#define array_val2(array, index, jndex) array_val(array_val(array, index), jndex)

#if 0
#ifdef DEBUG
#define val(array, index) (*(array_val_ptr(array, index)))
#else
#define val(array, index) (array)[index]
#endif
#endif

#define stack_begin_scope(stack_name) u64 stack_name = _stack_begin_scope()
#define stack_end_scope(stack_name) _stack_end_scope(stack_name);

#define stack_get() _stack_get(0)

typedef enum Allocation_Type
{
    MALLOC,
    ARRAY
} Allocation_Type;

typedef struct Region_Alloc
{
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


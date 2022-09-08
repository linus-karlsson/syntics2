#pragma once

#include "defines.h"

namespace synt {

    typedef enum Alloc_Type
    {
        TEMP_MALLOC,
        PERM_MALLOC,
        TEMP_ARRAY,
        PERM_ARRAY,
    } Alloc_Type;

    typedef struct Region_Alloc
    {
        Region_Alloc();
        ~Region_Alloc();

        unsigned char* buffer;
        u32 currentPos;
        u32 capacity;
        i32 types[4];
    } Region_Alloc;

    typedef struct Array_Head
    {
        u32 capacity;
        u32 size;
        u32 safetyFlag;
    } Array_Head;

    typedef struct Simple_Array_Head
    {
        u32 capacity;
        u32 size;
    } Simple_Array_Head;

#define syCAST(...) __VA_ARGS__

#define sy_size(array) sizeof(array) / sizeof(array[0])

#define region_malloc(region, num_elements, type, alloc_type)                       \
    (type*)synt::_region_malloc(region, (u32)(num_elements * sizeof(type)),         \
                                alloc_type)

#define region_pop(region, num_elements, type, alloc_type)                          \
    synt::_region_pop(region, num_elements * sizeof(type), alloc_type)

#define get_head(array) synt::_check_array(array)

#define dyn_array(region, capacity, type, alloc_type)                               \
    (type*)synt::_dyn_array(region, capacity, sizeof(type), alloc_type)

#define dyn_array_calloc(region, capacity, type, alloc_type)                        \
    (type*)synt::_dyn_array_calloc(region, capacity, sizeof(type), alloc_type)

#define dyn_array_val(region, extra_capacity, type, alloc_type, values)             \
    ({                                                                              \
        type in[] = { values };                                                     \
        (type*)synt::_dyn_array_val(region, (u32)(sizeof(in) / sizeof(type)),       \
                                    (u32)(sizeof(in) / sizeof(type)) +              \
                                        extra_capacity,                             \
                                    (u32)sizeof(type), alloc_type, in);             \
    })

#define dyn_array_copy(region, extra_capacity, type, values)                        \
    (type*)synt::_dyn_array_val(region, (u32)(sizeof(values) / sizeof(type)),       \
                                (u32)(sizeof(values) / sizeof(type)) +              \
                                    extra_capacity,                                 \
                                (u32)sizeof(type), values);

#define clear_arr(array, type) synt::_array_clear(array, sizeof(type))

#define synt_push(array, value)                                                     \
    {                                                                               \
        synt::Array_Head* head = synt::_check_array_push(array);                    \
        if (head) array[head->size++] = value;                                      \
    }

#define synt_push_unsafe(array, value)                                              \
    {                                                                               \
        synt::Array_Head* head = (((synt::Array_Head*)array) - 1);                  \
        array[head->size++]    = value;                                             \
    }

#define synt_pop(array)                                                             \
    ({                                                                              \
        synt::Array_Head* head = synt::_check_array_push(array);                    \
        head->size--;                                                               \
        array[head->size + 1];                                                      \
    })

#define _get_val_ptr(array, index)                                                  \
    synt::_check_array_size(array, index) ? (array + index) : 0

#define val(array, index) *(synt::_get_val_ptr(array, index))

#define region_pop_array(region, array, type)                                       \
    synt::_region_pop(region, ((get_head(array))->capacity * sizeof(type)) +        \
                                  sizeof(Array_Head))

    b8 init_region(Region_Alloc* region, u32 size);
    void* _region_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
    void _region_pop(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
    void reset_region(Region_Alloc* region);
    void free_region(Region_Alloc* region);
    void print_region(Region_Alloc* region);

    void* _dyn_array(Region_Alloc* region, u32 capacity, u32 type,
                     Alloc_Type alloc_type);
    void* _dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                            Alloc_Type alloc_type);

    void* _simple_dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                                   Alloc_Type alloc_type);

    void* _dyn_array_val(Region_Alloc* region, u32 num_elements, u32 capacity,
                         u32 type, Alloc_Type alloc_type, const void* values);

    Array_Head* _check_array(void* array);
    Array_Head* _check_array_push(void* array);
    b8 _check_array_size(void* array, u32 index);

    Array_Head* _dyn_check_array(void* array);
    Array_Head* _dyn_check_array_push(void* array);

    void _array_clear(void* array, u32 stride);
    void _push_back(void* array, void* value, u32 stride);

    u32 size_arr(const void* const array);
    u32 capacity_arr(const void* const array);

} // namespace synt

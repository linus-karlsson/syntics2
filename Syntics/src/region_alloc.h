#pragma once

#include "defines.h"
#include "logging.h"
#include "region_alloc.h"

#define stack_malloc(num_elements, type)                                            \
    (type*)_region_malloc(get_stack(), (u32)(num_elements * sizeof(type)),          \
                          TEMP_MALLOC)

#define stack_array(capacity, type)                                                 \
    (type*)_dyn_array(get_stack(), capacity, sizeof(type), TEMP_ARRAY, 0)

#define stack_calloc(num_elements, type)                                            \
    (type*)_region_calloc(get_stack(), (u32)(num_elements * sizeof(type)),          \
                          TEMP_MALLOC)

#define stack_array0(capacity, type)                                                \
    (type*)_dyn_array_calloc(get_stack(), capacity, sizeof(type), TEMP_ARRAY, 0)

#define stack_pop_malloc(num_elements, type)                                        \
    _region_pop(get_stack(), num_elements * sizeof(type), TEMP_MALLOC)

#define stack_pop_array(num_elements, type)                                         \
    _region_pop(get_stack(), num_elements * sizeof(type), TEMP_ARRAY)

#define region_malloc(region, num_elements, type, alloc_type)                       \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), alloc_type)

#define region_mallocP(region, num_elements, type)                                  \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), PERM_MALLOC)

#define region_malloc_struct(region, type)                                          \
    (type*)_region_malloc(region, (u32)(1 * sizeof(type)), PERM_MALLOC)

#define region_mallocT(region, num_elements, type)                                  \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), TEMP_MALLOC)

#define region_pop(region, num_elements, type, alloc_type)                          \
    _region_pop(region, num_elements * sizeof(type), alloc_type)

#define get_head(array) (((Array_Head*)(array)) - 1)

#define synt_back(array) ((array) + (get_head(array)->size - 1))

#define dyn_array(region, capacity, type, alloc_type)                               \
    (type*)_dyn_array(region, capacity, sizeof(type), alloc_type, 0)

#define dyn_arrayP(region, capacity, type)                                          \
    (type*)_dyn_array(region, capacity, sizeof(type), PERM_ARRAY, 0)

#define dyn_arrayT(region, capacity, type)                                          \
    (type*)_dyn_array(region, capacity, sizeof(type), TEMP_ARRAY, 0)

#define dyn_array_calloc(region, capacity, type, alloc_type)                        \
    (type*)_dyn_array_calloc(region, capacity, sizeof(type), alloc_type)

#define dyn_array_val(region, extra_capacity, type, alloc_type, values)             \
    ({                                                                              \
        type in[] = { values };                                                     \
        (type*)_dyn_array_val(region,                                               \
                              (u32)(sizeof(in) / sizeof(type)) + extra_capacity,    \
                              (u32)sizeof(type), alloc_type, in);                   \
    })

#define dyn_array_callocP(region, capacity, type)                                   \
    (type*)_dyn_array_calloc(region, capacity, sizeof(type), PERM_ARRAY);

#define dyn_array_valP(region, extra_capacity, type, values)                        \
    ({                                                                              \
        type in[] = { values };                                                     \
        (type*)_dyn_array_val(region, (u32)(sizeof(in) / sizeof(type)),             \
                              (u32)(sizeof(in) / sizeof(type)) + extra_capacity,    \
                              (u32)sizeof(type), PERM_ARRAY, in);                   \
    })

#define dyn_array_copy(region, extra_capacity, type, values)                        \
    (type*)_dyn_array_val(region, (u32)(sizeof(values) / sizeof(type)),             \
                          (u32)(sizeof(values) / sizeof(type)) + extra_capacity,    \
                          (u32)sizeof(type), values);

#define clear_arr(array, type) _array_clear(array, sizeof(type))

#define synt_push(array, value)                                                     \
    do                                                                              \
    {                                                                               \
        Array_Head* head = (((Array_Head*)(array)) - 1);                            \
        if (head && head->size < head->capacity)                                    \
            (array)[head->size++] = (value);                                        \
        else                                                                        \
            SY_ERROR("Array out of size!");                                         \
    } while (0)

#define synt_pop(array) (array)[_check_pop_array_size((array))]

#define get_val_ptr(array, index)                                                   \
    ((array) + _check_array_size_index((array), (index)))

#ifdef DEBUG
#define val(array, index) (*(get_val_ptr(array, index)))
#else
#define val(array, index) (array)[index]
#endif

typedef enum Alloc_Type
{
    TEMP_MALLOC,
    PERM_MALLOC,
    TEMP_ARRAY,
    PERM_ARRAY,
} Alloc_Type;

typedef struct Region_Alloc
{
    unsigned char* buffer;
    u64 currentPos;
    u64 capacity;
    i32 types[4];
} Region_Alloc;

Region_Alloc region_alloc(void);

typedef struct Array_Head
{
    Array_Head(u32 capacity, u32 size);
    u32 capacity;
    u32 size;
#ifdef DEBUG
    u64 safety_number();
private:
    u64 m_safety_number;
#endif
} Array_Head;

#define SCOPE(content)                                                              \
    {                                                                               \
        stack_begin_scope();                                                        \
        content stack_end_scope();                                                  \
    }

#define stack_begin_scope() u64 BEGIN_STACK_SCOPE_VAL_7891724 = _stack_begin_scope()
#define stack_end_scope() _stack_end_scope(BEGIN_STACK_SCOPE_VAL_7891724);

void init_stack(u32 size);
Region_Alloc* get_stack(void);
void reset_stack(void);
u64 _stack_begin_scope(void);
void _stack_end_scope(u64 size_at_start);

b8 init_region(Region_Alloc* region, u64 size);
void* _region_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
void* _region_calloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
void _region_pop(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
void reset_region(Region_Alloc* region);
void free_region(Region_Alloc* region);
void print_region(const Region_Alloc* region);

void* _dyn_array(Region_Alloc* region, u32 capacity, u32 type, Alloc_Type alloc_type,
                 u32 extra_size);

void* _dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type);

void* _simple_dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                               Alloc_Type alloc_type);

void* _dyn_array_val(Region_Alloc* region, u32 capacity, u32 type,
                     Alloc_Type alloc_type, const void* values);

u32 _check_array_size_index(void* array, u32 index);
b8 _check_array_size(void* array);
u32 _check_pop_array_size(void* array);

void _array_clear(void* array, u32 stride);
void _push_back(void* array, void* value, u32 stride);

u32 size_arr(const void* const array);
u32 capacity_arr(const void* const array);


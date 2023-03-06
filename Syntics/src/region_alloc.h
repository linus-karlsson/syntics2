#pragma once

#include "defines.h"
#include "logging.h"
#include <assert.h>
#include <string.h>

#define region_malloc(region, num_elements, type, alloc_type)                       \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), alloc_type);  \
    assert(!((region)->_count_check))

#define region_mallocP(region, num_elements, type)                                  \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), PERM_MALLOC); \
    assert(!((region)->_count_check))

#define region_mallocT(region, num_elements, type)                                  \
    (type*)_region_malloc(region, (u32)(num_elements * sizeof(type)), TEMP_MALLOC); \
    assert(!((region)->_count_check))

#define region_pop(region, num_elements, type, alloc_type)                          \
    _region_pop(region, num_elements * sizeof(type), alloc_type);

#define get_head(array) (((Array_Head*)array) - 1)

#define synt_back(array) (array + (get_head(array)->size - 1))

#define dyn_array(region, capacity, type, alloc_type)                               \
    (type*)_dyn_array(region, capacity, sizeof(type), alloc_type, 0);               \
    assert(!((region)->_count_check))

#define dyn_arrayP(region, capacity, type)                                          \
    (type*)_dyn_array(region, capacity, sizeof(type), PERM_ARRAY, 0);               \
    assert(!((region)->_count_check))

#define dyn_arrayT(region, capacity, type)                                          \
    (type*)_dyn_array(region, capacity, sizeof(type), TEMP_ARRAY, 0);               \
    assert(!((region)->_count_check))

#define dyn_array_calloc(region, capacity, type, alloc_type)                        \
    (type*)_dyn_array_calloc(region, capacity, sizeof(type), alloc_type);           \
    assert(!((region)->_count_check))

#define dyn_array_val(region, extra_capacity, type, alloc_type, values)             \
    ({                                                                              \
        assert(!((region)->_count_check));                                          \
        type in[] = { values };                                                     \
        (type*)_dyn_array_val(region, (u32)(sizeof(in) / sizeof(type)),             \
                              (u32)(sizeof(in) / sizeof(type)) + extra_capacity,    \
                              (u32)sizeof(type), alloc_type, in);                   \
    })

#define dyn_array_callocP(region, capacity, type)                                   \
    (type*)_dyn_array_calloc(region, capacity, sizeof(type), PERM_ARRAY);           \
    assert(!((region)->_count_check))

#define dyn_array_valP(region, extra_capacity, type, values)                        \
    ({                                                                              \
        assert(!((region)->_count_check));                                          \
        type in[] = { values };                                                     \
        (type*)_dyn_array_val(region, (u32)(sizeof(in) / sizeof(type)),             \
                              (u32)(sizeof(in) / sizeof(type)) + extra_capacity,    \
                              (u32)sizeof(type), PERM_ARRAY, in);                   \
    })

#define dyn_array_copy(region, extra_capacity, type, values)                        \
    (type*)_dyn_array_val(region, (u32)(sizeof(values) / sizeof(type)),             \
                          (u32)(sizeof(values) / sizeof(type)) + extra_capacity,    \
                          (u32)sizeof(type), values);                               \
    assert(!((region)->_count_check))

#define clear_arr(array, type) _array_clear(array, sizeof(type))

#define synt_push(array, value)                                                     \
    do                                                                              \
    {                                                                               \
        Array_Head* head = (((Array_Head*)array) - 1);                              \
        if (head && head->size < head->capacity)                                    \
            array[head->size++] = value;                                            \
        else                                                                        \
            SY_ERROR("Array out of size!");                                         \
    } while (0)

// TODO: find a way to fix this on windows
#define synt_pop(array)                                                             \
    (Array_Head* head = (((Array_Head*)array) - 1); array[head->size--];)

#define _get_val_ptr(array, index)                                                  \
    _check_array_size(array, index) ? (array + index) : 0

#define val(array, index) *(_get_val_ptr(array, index))

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
    u64 currentPos;
    u64 capacity;
    i32 types[4];

    u32 _count_check;

} Region_Alloc;

typedef struct Array_Head
{
    u32 capacity;
    u32 size;
} Array_Head;

b8 init_region(Region_Alloc* region, u64 size);
void* _region_malloc(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
void _region_pop(Region_Alloc* region, u32 size, Alloc_Type alloc_type);
void reset_region(Region_Alloc* region);
void free_region(Region_Alloc* region);
void print_region(const Region_Alloc& region);

void* _dyn_array(Region_Alloc* region, u32 capacity, u32 type, Alloc_Type alloc_type,
                 u32 extra_size);

void* _dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                        Alloc_Type alloc_type);

void* _simple_dyn_array_calloc(Region_Alloc* region, u32 capacity, u32 type,
                               Alloc_Type alloc_type);

void* _dyn_array_val(Region_Alloc* region, u32 num_elements, u32 capacity, u32 type,
                     Alloc_Type alloc_type, const void* values);

b8 _check_array_size(void* array, u32 index);

void _array_clear(void* array, u32 stride);
void _push_back(void* array, void* value, u32 stride);

u32 size_arr(const void* const array);
u32 capacity_arr(const void* const array);

u32 _get_id();

template <typename T>
struct Temp_Alloc
{
    Temp_Alloc() : data(0), region_ref(0)
    {
    }
    Temp_Alloc(Region_Alloc* region, u32 num_elements)
        : data((T*)_dyn_array(region, num_elements, sizeof(T), TEMP_ARRAY, 0)),
          region_ref(region), temp_id(_get_id())
    {
#if 0
        synt_LOG("%sINIT%s Temp_Alloc ID: %u SIZE: %u\n", ANSI_COLOR_GREEN,
                 ANSI_COLOR_RESET, temp_id, num_elements);
#endif
        region->_count_check++;
    }
    ~Temp_Alloc()
    {
        if (region_ref)
        {
#if 0
            synt_LOG("%sDEL%s Temp_Alloc ID: %u SIZE: %u\n", ANSI_COLOR_RED,
                     ANSI_COLOR_RESET, temp_id, capacity_arr(data));
#endif

            region_pop(region_ref, capacity_arr(data), T, TEMP_ARRAY);
            region_ref->_count_check--;
            region_ref = NULL;
        }
    }
    void init(Region_Alloc* region, u32 num_elements)
    {
        assert(!data);

        temp_id = _get_id();

#if 0
        synt_LOG("%sINIT%s Temp_Alloc ID: %u SIZE: %u\n", ANSI_COLOR_GREEN,
                 ANSI_COLOR_RESET, temp_id, num_elements);
#endif
        data = (T*)_dyn_array(region, num_elements, sizeof(T), TEMP_ARRAY, 0),
        region_ref = region;
        region->_count_check++;
    }
    u32 capacity()
    {
        return capacity_arr(data);
    }
    u32 capacity() const
    {
        return capacity_arr(data);
    }
    u32 size()
    {
        return size_arr(data);
    }
    u32 size() const
    {
        return size_arr(data);
    }
    void push_back(T value)
    {
        synt_push(data, value);
    }
    T pop()
    {
        Array_Head* head = (((Array_Head*)data) - 1);
        if (head->size > 0) return data[head->size--];
    }
    void destroy()
    {
        this->~Temp_Alloc();
    }

    T* data;
    Region_Alloc* region_ref;

private:
    u32 temp_id;
};

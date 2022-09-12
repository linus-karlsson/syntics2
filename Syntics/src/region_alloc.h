#pragma once

#include "defines.h"
#include "logging.h"
#include <assert.h>

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
        uint32 currentPos;
        uint32 capacity;
        int32 types[4];
    } Region_Alloc;

    typedef struct Array_Head
    {
        uint32 capacity;
        uint32 size;
        uint32 safetyFlag;
    } Array_Head;

    typedef struct Simple_Array_Head
    {
        uint32 capacity;
        uint32 size;
    } Simple_Array_Head;

#define sy(...) __VA_ARGS__

#define sy_size(array) sizeof(array) / sizeof(array[0])

#define region_malloc(region, num_elements, type, alloc_type)                       \
    (type*)synt::_region_malloc(region, (uint32)(num_elements * sizeof(type)),      \
                                alloc_type)

    // Single temporary malloc
#define region_ST(region, type)                                                     \
    (type*)synt::_region_malloc(region, (uint32)(1 * sizeof(type)),                 \
                                synt::TEMP_MALLOC)

    // Single perm malloc
#define region_SP(region, type)                                                     \
    (type*)synt::_region_malloc(region, (uint32)(1 * sizeof(type)),                 \
                                synt::PERM_MALLOC)

    // Single temporary malloc value
#define region_STV(region, type, value)                                             \
    &(*((type*)synt::_region_malloc(region, (uint32)(1 * sizeof(type)),             \
                                    synt::TEMP_MALLOC)) = value);

    // Single perm malloc value
#define region_SPV(region, type, value)                                             \
    &(*((type*)synt::_region_malloc(region, (uint32)(1 * sizeof(type)),             \
                                    synt::PERM_MALLOC)) = value);

#define region_pop(region, num_elements, type, alloc_type)                          \
    synt::_region_pop(region, num_elements * sizeof(type), alloc_type)

#define region_SPOP(region, type)                                                   \
    synt::_region_pop(region, 1 * sizeof(type), synt::TEMP_MALLOC)

#define get_head(array) synt::_check_array(array)

#define dyn_array(region, capacity, type, alloc_type)                               \
    (type*)synt::_dyn_array(region, capacity, sizeof(type), alloc_type, 0)

#define dyn_array_calloc(region, capacity, type, alloc_type)                        \
    (type*)synt::_dyn_array_calloc(region, capacity, sizeof(type), alloc_type)

#define dyn_array_val(region, extra_capacity, type, alloc_type, values)             \
    ({                                                                              \
        type in[] = { values };                                                     \
        (type*)synt::_dyn_array_val(region, (uint32)(sizeof(in) / sizeof(type)),    \
                                    (uint32)(sizeof(in) / sizeof(type)) +           \
                                        extra_capacity,                             \
                                    (uint32)sizeof(type), alloc_type, in);          \
    })

#define dyn_array_copy(region, extra_capacity, type, values)                        \
    (type*)synt::_dyn_array_val(region, (uint32)(sizeof(values) / sizeof(type)),    \
                                (uint32)(sizeof(values) / sizeof(type)) +           \
                                    extra_capacity,                                 \
                                (uint32)sizeof(type), values);

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

    bool init_region(Region_Alloc* region, uint32 size);
    void* _region_malloc(Region_Alloc* region, uint32 size, Alloc_Type alloc_type);
    void _region_pop(Region_Alloc* region, uint32 size, Alloc_Type alloc_type);
    void reset_region(Region_Alloc* region);
    void free_region(Region_Alloc* region);
    void print_region(const Region_Alloc& region);

    void* _dyn_array(Region_Alloc* region, uint32 capacity, uint32 type,
                     Alloc_Type alloc_type, uint32 extra_size);

    void* _dyn_array_calloc(Region_Alloc* region, uint32 capacity, uint32 type,
                            Alloc_Type alloc_type);

    void* _simple_dyn_array_calloc(Region_Alloc* region, uint32 capacity,
                                   uint32 type, Alloc_Type alloc_type);

    void* _dyn_array_val(Region_Alloc* region, uint32 num_elements, uint32 capacity,
                         uint32 type, Alloc_Type alloc_type, const void* values);

    Array_Head* _check_array(void* array);
    Array_Head* _check_array_push(void* array);
    bool _check_array_size(void* array, uint32 index);

    Array_Head* _dyn_check_array(void* array);
    Array_Head* _dyn_check_array_push(void* array);

    void _array_clear(void* array, uint32 stride);
    void _push_back(void* array, void* value, uint32 stride);

    uint32 size_arr(const void* const array);
    uint32 capacity_arr(const void* const array);

    uint32 _get_id();

    template <typename T>
    struct Temp_Alloc
    {
        Temp_Alloc() : data(0), region_ref(0) {}
        Temp_Alloc(Region_Alloc* region, uint32 num_elements)
            : temp_id(_get_id()),
              data(dyn_array(region, num_elements, T, TEMP_ARRAY)),
              region_ref(region)
        {
            synt_LOG("INIT Temp_Alloc ID: %u SIZE: %u\n", temp_id, num_elements);
        }
        ~Temp_Alloc()
        {
            synt_LOG("DEL Temp_Alloc ID: %u SIZE: %u\n", temp_id,
                     capacity_arr(data));

            if (region_ref)
                region_pop(region_ref, capacity_arr(data), T, TEMP_ARRAY);
        }
        void init(Region_Alloc* region, uint32 num_elements)
        {
            assert(!data);

            temp_id = _get_id();

            synt_LOG("INIT Temp_Alloc ID: %u SIZE: %u\n", temp_id, num_elements);

            data       = dyn_array(region, num_elements, T, TEMP_ARRAY);
            region_ref = region;
        }

        Region_Alloc* region_ref;
        T* data;

    private:
        uint32 temp_id;
    };

} // namespace synt

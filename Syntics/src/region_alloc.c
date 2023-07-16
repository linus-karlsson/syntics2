
global Region_Alloc REGION_g_stack = NULL;

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
    Region_Alloc_Internal* region_internal =
        (Region_Alloc_Internal*)calloc(1, sizeof(Region_Alloc_Internal));
    assert(region_internal);

#ifdef LINUX
    region_internal->buffer = (unsigned char*)mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (region_internal->buffer == MAP_FAILED) ERROR("init_region");
#else
#if 1
    region_internal->buffer = (unsigned char*)VirtualAlloc(
        0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif

#if 0
        region_internal->buffer = (unsigned char*)calloc(size, 1);
        if (region_internal->buffer == NULL) SY_ERROR("init_region");
#endif

#endif

    region_internal->capacity = size;
    region_internal->currentPos = 0;

    *region = region_internal;

    return 1;
}

void stack_init(u32 size)
{
    region_init(&REGION_g_stack, size);
}

Region_Alloc _stack_get(u32 check_val)
{
    return REGION_g_stack;
}

void stack_reset(void)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)REGION_g_stack;

    region_internal->currentPos = 0;
}

u64 _stack_begin_scope(void)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)REGION_g_stack;

    return region_internal->currentPos;
}

global u64 g_biggest_stack_size = 0;

#define MAX(val1, val2) ((val1) > (val2) ? (val1) : (val2))

void _stack_end_scope(u64 size_at_start)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)REGION_g_stack;

    g_biggest_stack_size = MAX(g_biggest_stack_size, region_internal->currentPos);
    region_internal->currentPos = size_at_start;
}

Region_Alloc region_alloc(void)
{
    Region_Alloc res = { 0 };
    return res;
}

static void* malloc_init(Region_Alloc region, u32 size)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)region;
    assert(region_internal);

    assert(size < region_internal->capacity - region_internal->currentPos &&
           "iarray_nit_not enough memory");

    unsigned char* currentPos =
        region_internal->buffer + region_internal->currentPos;
    region_internal->currentPos += size;
    return currentPos;
}

void* _region_malloc(Region_Alloc region, u32 size)
{
    return malloc_init(region, size);
}

void* _region_calloc(Region_Alloc region, u32 size)
{
    void* res = malloc_init(region, size);
    memset(res, 0, size);
    return res;
}

void _region_pop(Region_Alloc region, u32 size, Allocation_Type alloc_type)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)region;
    assert(region_internal);

    if (size > region_internal->currentPos)
    {
        region_internal->currentPos = 0;
    }
    else if (alloc_type == ARRAY)
    {
        region_internal->currentPos -= (size + sizeof(Array_Head));
    }
    else
    {
        region_internal->currentPos -= size;
    }
}

void region_reset(Region_Alloc region)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)region;
    assert(region_internal);

    region_internal->currentPos = 0;
}

#if 1
void region_free(Region_Alloc region)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)region;
    assert(region_internal);

    free(region_internal->buffer);
    free(region);
}
#endif

void region_print(const Region_Alloc region)
{
    const Region_Alloc_Internal* region_internal =
        (const Region_Alloc_Internal*)region;
    assert(region_internal);
#if 0
    printf("\n");
    synt_LOG("%sTotal memory:%s %llu\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             region.capacity);
    synt_LOG("%sTotal memory used:%s %llu\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             region.currentPos);
    synt_LOG("%sTotal memory left:%s %llu\n", ANSI_COLOR_MAGENTA, ANSI_COLOR_RESET,
             region.capacity - region.currentPos);

    printf("\n");
    synt_LOG("%sPERM Malloc allocations:%s %d\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             (region.types[PERM_MALLOC]));
    synt_LOG("%sPERM Array allocations:%s %d\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET,
             (region.types[PERM_ARRAY]));
    synt_LOG("%sTEMP Malloc allocations:%s %d\n", ANSI_COLOR_MAGENTA,
             ANSI_COLOR_RESET, (region.types[TEMP_MALLOC]));
    synt_LOG("%sTEMP Array allocations:%s %d\n\n", ANSI_COLOR_MAGENTA,
             ANSI_COLOR_RESET, (region.types[TEMP_ARRAY]));
#endif
    static int count = 0;
    sy_print("\ncount: %d\n", count++);
    sy_print("Total memory: %llu\n", region_internal->capacity);
    sy_print("Total memory used: %llu\n", region_internal->currentPos);
    sy_print("Total memory left: %llu\n",
             region_internal->capacity - region_internal->currentPos);

    sy_print("Biggest stack: %llu\n", g_biggest_stack_size);
}

static void* array_init(Region_Alloc region, u32 capacity, u32 type, 
                        u32 extra_size)
{
    Region_Alloc_Internal* region_internal = (Region_Alloc_Internal*)region;
    assert(region_internal);

    const u32 size = capacity * type;

    assert((size < region_internal->capacity - region_internal->currentPos) &&
           "init array Not enough memory");

    Array_Head* head_pos =
        (Array_Head*)(region_internal->buffer + region_internal->currentPos);
    *head_pos = array_head_create(capacity, 0);
    head_pos++;

    region_internal->currentPos += (size + sizeof(Array_Head) + extra_size);

    return (void*)head_pos;
}

void* _region_array(Region_Alloc region, u32 capacity, u32 type, u32 extra_size)
{
    return array_init(region, capacity, type, extra_size);
}
void* _region_array_calloc(Region_Alloc region, u32 capacity, u32 type)
{
    const u32 size = capacity * type;
    void* head_pos = array_init(region, capacity, type, 0);
    memset(head_pos, 0, size);
    return head_pos;
}

void* _region_array_val(Region_Alloc region, u32 capacity, u32 type, 
                        const void* values)
{
    const u32 size = capacity * type;
    void* head_pos = array_init(region, capacity, type, 0);
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
    assert(head->_safety_number == REGION_CHECK_VALUE && "Array Do not have a size");
    if (head->size < head->capacity)
    {
        return true;
    }
    return false;
}

u32 _array_check_size_index(void* array, u32 index)
{
    Array_Head* head = (((Array_Head*)(array)) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE && "Array Do not have a size");
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
    assert(head->_safety_number == REGION_CHECK_VALUE && "Array Do not have a size");
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
    assert(head->_safety_number == REGION_CHECK_VALUE && "Array Do not have a size");

    return head->size;
}

u32 array_capacity(const void* const array)
{
    Array_Head* head = (((Array_Head*)array) - 1);
    assert(head->_safety_number == REGION_CHECK_VALUE && "Array Do not have a size");

    return head->capacity;
}

#define path_extend_d0(region, path) path_extend(region, path, (u32)strlen(path))
#define path_extend_d1(path) path_extend(stack_get(), path, (u32)strlen(path))
char* path_extend(Region_Alloc region, const char* trailing_path,
                  u32 trailing_path_len)
{
    char* result =
        region_array(region, WORKING_DIR_LEN + trailing_path_len + 1, char);
    memcpy(result, WORKING_DIR, WORKING_DIR_LEN);
    memcpy(result + WORKING_DIR_LEN, trailing_path, trailing_path_len);
    val(result, WORKING_DIR_LEN + trailing_path_len) = '\0';
    return result;
}

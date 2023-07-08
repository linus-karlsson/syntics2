#pragma once
//#include "defines.h"

typedef struct Table_Row
{
    u32 index;
    u32 ref_value;
} Table_Row;

typedef struct Lookup_Key
{
    Table_Row _row;
} Lookup_Key;

typedef struct Lookup_Table
{
    Table_Row* _entries;
    u32* _free_indices;
    u32 _num_free_indices;
    u32 _num_entries;
} Lookup_Table;


#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

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

Lookup_Table lookup_table_create(Region_Alloc* region, u32 n_entries);
Lookup_Key entry_add(Lookup_Table* table, u32 ref_index);
u32 table_index(Lookup_Table* table, Lookup_Key key);
u32 entry_remove(Lookup_Table* table, Lookup_Key key);
void entry_index_change(Lookup_Table* table, u32 entry, u32 new_index);

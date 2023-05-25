#pragma once
#include "defines.h"

typedef struct Table_Row
{
    u32 index;
    u32 ref_value;
} Table_Row;

typedef struct Lookup_Key
{
    Lookup_Key(Table_Row row);
    u32 table_index();
    u32 ref_value();

private:
    Table_Row m_row;
} Lookup_Key;

typedef struct Lookup_Table
{
    Lookup_Table(Region_Alloc* region, u32 n_entries);
    Lookup_Key add_entry(u32 ref_index);
    u32 index(Lookup_Key key);
    u32 remove_entry(Lookup_Key key);
    void cange_entry_index(u32 entry, u32 new_index);

private:
    Table_Row* m_entries;
    u32* m_free_indices;
    u32 m_num_free_indices;
    u32 m_num_entries;
} Lookup_Table;

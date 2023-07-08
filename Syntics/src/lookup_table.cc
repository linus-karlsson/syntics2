#include "lookup_table.h"
#include "region_alloc.h"
#include "logging.h"

Lookup_Key::Lookup_Key(Table_Row row) : m_row(row)
{
}

u32 Lookup_Key::table_index()
{
    return m_row.index;
}

u32 Lookup_Key::ref_value()
{
    return m_row.ref_value;
}

Lookup_Table::Lookup_Table(Region_Alloc* region, u32 n_entries)
{
    m_num_entries = 1;
    m_num_free_indices = 0;
    m_entries = dyn_array_callocP(region, n_entries + 1, Table_Row);
    m_free_indices = dyn_array_callocP(region, n_entries + 1, u32);
}

Lookup_Key Lookup_Table::add_entry(u32 ref_index)
{
    u32 capacity = capacity_arr(m_entries);
    ASSERT(m_num_entries < capacity, "add_dyn_entity");

    u32 index = 0;

    u32 free_size = size_arr(m_free_indices);
    if (free_size)
    {
        index = synt_pop(m_free_indices);
        m_num_free_indices--;
    }
    else
    {
        index = m_num_entries;
    }
    Table_Row row = { index, m_entries[index].ref_value };
    Lookup_Key out(row);

    val(m_entries, index).index = ref_index;

    m_num_entries++;

    return out;
}

u32 Lookup_Table::index(Lookup_Key key)
{
    ASSERT(key.table_index() < m_num_entries, "Lookup_Table index");

    Table_Row* current_row = m_entries + key.table_index();

    u32 out = 0;
    if (current_row->index != 0 && current_row->ref_value == key.ref_value())
    {
        out = current_row->index;
    }
    return out;
}

u32 Lookup_Table::remove_entry(Lookup_Key key)
{
    u32 result = 0;

    u32 capacity = capacity_arr(m_entries);
    ASSERT(key.table_index() < capacity, "remove_dyn_entitiy e._table_index");
    if (key.table_index() == 0) return result;

    Table_Row* current_row = m_entries + key.table_index();
    if (current_row->index != 0 && current_row->ref_value == key.ref_value())
    {
        current_row->ref_value++;
        ASSERT(current_row->ref_value < U32_MAX - 10, "ref_value is to large");

        synt_push(m_free_indices, key.table_index());

        m_num_entries--;
        result = current_row->index;
        current_row->index = 0;
    }
    return result;
}

void Lookup_Table::cange_entry_index(u32 entry, u32 new_index)
{
    val(m_entries, entry).index = new_index;
}

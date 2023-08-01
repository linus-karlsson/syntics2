
Lookup_Table lookup_table_create(Region_Alloc* region, u32 n_entries)
{
    Lookup_Table out;
    out._num_entries = 1;
    out._num_free_indices = 0;
    if (n_entries)
    {
        out._entries = region_array_calloc(region, n_entries + 1, Table_Row);
        out._free_indices = region_array_calloc(region, n_entries + 1, u32);
    }
    return out;
}

Lookup_Key entry_add(Lookup_Table* table, u32 ref_index)
{
    u32 capacity = array_capacity(table->_entries);
    assert(table->_num_entries < capacity);

    u32 index = 0;

    u32 free_size = array_size(table->_free_indices);
    if (free_size)
    {
        index = array_pop(table->_free_indices);
        table->_num_free_indices--;
    }
    else
    {
        index = table->_num_entries;
    }
    Table_Row row = { index, table->_entries[index].ref_value };
    Lookup_Key out = { row };

    array_val(table->_entries, index).index = ref_index + 1;

    table->_num_entries++;

    return out;
}

u32 table_index(Lookup_Table* table, Lookup_Key key)
{
    assert(key._row.index < table->_num_entries);

    Table_Row* current_row = table->_entries + key._row.index;

    u32 out = 0;
    if (current_row->index != 0 && current_row->ref_value == key._row.ref_value)
    {
        out = current_row->index;
    }
    return out;
}

u32 entry_remove(Lookup_Table* table, Lookup_Key key)
{
    u32 result = 0;

    u32 capacity = array_capacity(table->_entries);
    assert(key._row.index < capacity);
    if (key._row.index == 0) return result;

    Table_Row* current_row = table->_entries + key._row.index;
    if (current_row->index != 0 && current_row->ref_value == key._row.ref_value)
    {
        current_row->ref_value++;
        assert(current_row->ref_value < U32_MAX - 10);

        array_push(table->_free_indices, key._row.index);

        table->_num_entries--;
        result = current_row->index;
        current_row->index = 0;
    }
    return result;
}

void entry_index_change(Lookup_Table* table, u32 entry, u32 new_index)
{
    array_val(table->_entries, entry).index = new_index;
}

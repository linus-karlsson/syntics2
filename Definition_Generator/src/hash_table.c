

// Murmurhash3
u32 hash_function(const char* key, u32 len, u32 capacity, u32 seed)
{
    u32 h = seed;
    if (len > 3)
    {
        const u32* key_x4 = (const u32*)key;
        u32 i, n = len >> 2;
        for (i = 0; i < n; i++)
        {
            u32 k = key_x4[i];
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;
            h ^= k;
            h = (h << 13) | (h >> 19);
            h = (h * 5) + 0xe6546b64;
        }
        key = (const char*)(key_x4 + n);
        n = len & 3;
    }

    u32 k1 = 0;
    switch (len)
    {
        case 3:
        {
            k1 ^= key[2] << 16;
        }
        case 2:
        {
            k1 ^= key[1] << 8;
        }
        case 1:
        {
            k1 ^= key[0];
            k1 *= 0xcc9e2d51;
            k1 = (k1 << 15) | (k1 >> 17);
            k1 *= 0x1b873593;
            h ^= k1;
        }
    }

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    h %= (capacity - 1);
    return h;
}

Hash_Table_String hash_table_string_create(u32 capacity,
                                           u32 collision_buffer_capacity)
{
    Hash_Table_String out = { 0 };
    out.capacity = capacity;
    out.values = (Node_String*)calloc(capacity, sizeof(Node_String));
    out.collision_buffer_capacity = collision_buffer_capacity;
    out.collision_buffer =
        (Node_String*)calloc(collision_buffer_capacity, sizeof(Node_String));
    return out;
}

Node_String* next_node_string(Hash_Table_String* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return table->collision_buffer + table->collision_buffer_size++;
}

void insert_value_string(Hash_Table_String* table, const char* key,
                         const char* value)
{
    u32 key_len = (u32)strlen(key);
    Node_String* node =
        table->values + hash_function(key, key_len, table->capacity, table->seed);
    if (node->value)
    {
        if (!strcmp(node->value, value))
        {
            return;
        }
        while (node->next)
        {
            if (!strcmp(node->value, value))
            {
                return;
            }
            node = node->next;
        }
        node->next = next_node_string(table);
        node = node->next;
    }
    // Don't know how to make this not use calloc
    size_t len = strlen(value) + 1;
    node->value = (char*)calloc(len, 1);
    memcpy(node->value, value, len);
}

char* get_value_string(Hash_Table_String* table, const char* key)
{
    u32 key_len = (u32)strlen(key);
    Node_String* node =
        table->values + hash_function(key, key_len, table->capacity, table->seed);
    if (node->value)
    {
        return node->value;
    }
    else
    {
        return NULL;
    }
}

void free_hash_table_string(Hash_Table_String* table)
{
    for (u32 i = 0; i < table->capacity; i++)
    {
        Node_String* node = table->values + i;
        if (node)
        {
            if (node->value)
            {
                free(node->value);
                while (node->next)
                {
                    node = node->next;
                    if (node->value)
                    {
                        free(node->value);
                    }
                }
            }
        }
    }
    free(table->values);
    free(table->collision_buffer);
}

Hash_Table_U32 hash_table_u32_create(u32 capacity, u32 collision_buffer_capacity)
{
    Hash_Table_U32 out = { 0 };
    out.capacity = capacity;
    out.values = (Node_U32*)calloc(capacity, sizeof(Node_U32));
    out.collision_buffer_capacity = collision_buffer_capacity;
    out.collision_buffer =
        (Node_U32*)calloc(collision_buffer_capacity, sizeof(Node_U32));
    return out;
}

Node_U32* next_node_u32(Hash_Table_U32* table)
{
    assert(table->collision_buffer_size < table->collision_buffer_capacity);
    return table->collision_buffer + table->collision_buffer_size++;
}

void insert_value_u32(Hash_Table_U32* table, const char* key, u32 value)
{
    u32 key_len = (u32)strlen(key);
    Node_U32* node =
        table->values + hash_function(key, key_len, table->capacity, table->seed);
    if (node->active)
    {
        if (node->value == value)
        {
            return;
        }
        while (node->next)
        {
            if (node->active)
            {
                if (node->value == value)
                {
                    return;
                }
            }
            node = node->next;
        }
        node->next = next_node_u32(table);
        node = node->next;
    }
    node->value = value;
    node->active = true;
}

u32* get_value_u32(Hash_Table_U32* table, const char* key)
{
    u32 key_len = (u32)strlen(key);

    Node_U32* node =
        table->values + hash_function(key, key_len, table->capacity, table->seed);

    if (node->active)
    {
        return &node->value;
    }
    else
    {
        return NULL;
    }
}

void free_hash_table_u32(Hash_Table_U32* table)
{
    free(table->values);
    free(table->collision_buffer);
}

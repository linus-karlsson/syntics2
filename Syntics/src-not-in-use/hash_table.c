
// Murmurhash3
u32 hash_function(const char* key, u32 len, u32 seed)
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
        n   = len & 3;
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

    h %= (HASH_TABLE_CAPACITY - 1);
    return h;
}

u32 seed = 0;

static u32 g_counter = 0;
void insert_value(HashTable* hash_table, const char* key, const char* value)
{
    u32 key_len = (u32)strlen(key);
    Node* node  = hash_table->values + hash_function(key, key_len, 0);
    if (node->value)
    {
        if (!strcmp(node->value, value))
        {
            return;
        }
        g_counter++;
        while (node->next)
        {
            if (!strcmp(node->value, value))
            {
                return;
            }
            node = node->next;
        }
        node->next = (Node*)calloc(1, sizeof(Node));
        node       = node->next;
    }
    size_t len  = strlen(value) + 1;
    node->value = (char*)calloc(len, 1);
    memcpy(node->value, value, len);
}

char* get_value(HashTable* ht, const char* key)
{
    u32 key_len = (u32)strlen(key);
    Node* node  = ht->values + hash_function(key, key_len, 0);
    if (node->value)
    {
        return node->value;
    } else
    {
        return "";
    }
}

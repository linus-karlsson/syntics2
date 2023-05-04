#pragma once

#include "defines.h"

#define HASH_TABLE_CAPACITY 1000 // make a better plan

typedef struct Node Node;
struct Node
{
    Node* next;
    char* value;
};

typedef struct HashTable
{
    Node* values;
    u32 size;
    u32 collisions;
} HashTable;

void init_hash_table(HashTable* ht);
u32 hash_function(const char* key, u32 len, u32 seed);
void insert_value(HashTable* hash_table, const char* key, const char* value);
char* get_value(HashTable* ht, const char* key);



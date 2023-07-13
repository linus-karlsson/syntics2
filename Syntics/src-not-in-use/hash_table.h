#pragma once

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


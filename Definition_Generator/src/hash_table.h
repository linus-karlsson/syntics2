#pragma once

typedef struct Node_String Node_String;
struct Node_String
{
    Node_String* next;
    char* value;
};

typedef struct Hash_Table_String
{
    Node_String* values;
    u32 capacity;

    Node_String* collision_buffer;
    u32 collision_buffer_size;
    u32 collision_buffer_capacity;

    u32 seed;
} Hash_Table_String;

typedef struct Node_U32 Node_U32;
struct Node_U32
{
    Node_U32* next;
    u32 value;
    u32 active;
};

typedef struct Hash_Table_U32
{
    Node_U32* values;
    u32 capacity;

    Node_U32* collision_buffer;
    u32 collision_buffer_size;
    u32 collision_buffer_capacity;

    u32 seed;
} Hash_Table_U32;

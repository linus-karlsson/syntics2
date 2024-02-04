#pragma once
#ifndef SY_UNIT_BUILD
#include "defines.h"
#endif

typedef struct Thread_Task
{
    void (*task_callback)(void* data);
    void* data;
} Thread_Task;

typedef struct Thread_Task_Internal
{
    Thread_Task task;
    Semaphore* sempahore;
} Thread_Task_Internal;

typedef struct Semaphore_Counter
{
    Semaphore* sempahore;
    u32 count;
} Semaphore_Counter;

typedef struct Thread_Task_Queue
{
    Semaphore mutex;
    Semaphore start_semaphore;
    Thread_Task_Internal* tasks;
    u32 capacity;
    volatile u32 size;
    volatile u32 head;
    volatile u32 tail;
} Thread_Task_Queue;

typedef struct Thread_Attrib
{
    Semaphore* start_semaphore;
    Thread_Task_Queue* queue;
    u32 id;
} Thread_Attrib;

typedef struct Thread_Queue
{
    Thread_Handle* pool;
    Thread_Attrib* attribs;
    Thread_Task_Queue task_queue;
} Thread_Queue;

#define THREAD_TASK_ENTRY_POINT(function_name) void function_name(void* data)
Thread_Task 
     thread_task(void (*task_callback)(void* data), void* data);
void semaphore_counter_wait(Semaphore_Counter* semaphore_counter);
void semaphore_counter_wait_and_free(Semaphore_Counter* semaphore_counter);
void thread_tasks_push(Region_Alloc* region, Thread_Task_Queue* task_queue, Thread_Task* tasks, u32 task_count, Semaphore_Counter* semaphore_counter);
void thread_init(Region_Alloc* region, u32 capacity, u32 thread_count, Thread_Queue* queue);
void threads_destroy(Thread_Queue* queue);

extern u32 global_thread_count;

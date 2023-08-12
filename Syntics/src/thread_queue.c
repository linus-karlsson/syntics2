#ifndef SY_INCLUDES // only for clangd
#include "syntics.h"
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
    Mutex mutex;
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

// TODO: only one queue for the moment
global Thread_Handle* thread_pool;
global Thread_Attrib* thread_attribs;
global Thread_Task_Queue thread_task_queue = { 0 };

// TODO: Fibers and spin locks instead of semaphores
// global _Atomic u32 atomic_counter = 0;
// spin lock :
//      while(counter != value);

#define THREAD_TASK_ENTRY_POINT(function_name) void function_name(void* data)

Thread_Task thread_task(void (*task_callback)(void* data), void* data)
{
    Thread_Task task = { .task_callback = task_callback, .data = data };
    return task;
}

void semaphore_counter_wait(Semaphore_Counter* semaphore_counter)
{
    if (semaphore_counter->count)
    {
        assert(semaphore_counter->sempahore);
        for (u32 i = 0; i < semaphore_counter->count; i++)
        {
            semaphore_wait_and_decrement(semaphore_counter->sempahore);
        }
        semaphore_counter->count = 0;
    }
}

void semaphore_counter_wait_and_free(Semaphore_Counter* semaphore_counter)
{
    semaphore_counter_wait(semaphore_counter);
    semaphore_destroy(semaphore_counter->sempahore);
    free(semaphore_counter->sempahore);
}

void _thread_task_push(Thread_Task task, Semaphore* semaphore)
{
    mutex_lock(&thread_task_queue.mutex);

    assert(thread_task_queue.size < thread_task_queue.capacity);

    thread_task_queue.tail %= thread_task_queue.capacity;
    thread_task_queue.tasks[thread_task_queue.tail] =
        (Thread_Task_Internal){ .task = task, .sempahore = semaphore };

    // printf("Tail: %u\n", thread_task_queue.tail);
    thread_task_queue.tail++;
    thread_task_queue.size++;

    mutex_unlock(&thread_task_queue.mutex);

    semaphore_increment(&thread_task_queue.start_semaphore);
}

void thread_tasks_push(Thread_Task* tasks, u32 task_count,
                       Semaphore_Counter* semaphore_counter)
{
    if (semaphore_counter)
    {
        if (!semaphore_counter->sempahore && task_count)
        {
            semaphore_counter->sempahore = (Semaphore*)calloc(1, sizeof(Semaphore));
            *semaphore_counter->sempahore = semaphore_create(0, task_count);
        }
        semaphore_counter->count = task_count;
    }
    for (u32 i = 0; i < task_count; i++)
    {
        _thread_task_push(tasks[i], semaphore_counter->sempahore);
    }
}

Thread_Task_Internal thread_task_pop()
{
    mutex_lock(&thread_task_queue.mutex);

    assert(thread_task_queue.size > 0);

    thread_task_queue.head %= thread_task_queue.capacity;
    Thread_Task_Internal task = thread_task_queue.tasks[thread_task_queue.head];

    // printf("Head: %u\n", thread_task_queue.head);
    thread_task_queue.head++;
    thread_task_queue.size--;

    mutex_unlock(&thread_task_queue.mutex);
    return task;
}

thread_return_value thread_loop(void* data)
{
    Thread_Attrib* attrib = (Thread_Attrib*)data;

    for (;;)
    {
        semaphore_wait_and_decrement(attrib->start_semaphore);

        // printf("Thread %u start\n",attrib->id);

        Thread_Task_Internal task = thread_task_pop();
        task.task.task_callback(task.task.data);

        // printf("Thread %u end\n",attrib->id);


        if (task.sempahore)
        {
            semaphore_increment(task.sempahore);
        }
    }
}

void thread_init(Region_Alloc* region, u32 capacity, u32 thread_count)
{
    assert(!thread_pool);
    if (thread_count > 8)
    {
        thread_count = 8;
    }
    thread_pool = region_array_calloc(region, thread_count, Thread_Handle);
    thread_attribs = region_calloc(region, thread_count, Thread_Attrib);

    Semaphore start_semaphore = semaphore_create(0, capacity);
    Mutex mutex = mutex_create();
    thread_task_queue.start_semaphore = start_semaphore;
    thread_task_queue.mutex = mutex;
    thread_task_queue.capacity = capacity;
    thread_task_queue.tasks =
        region_calloc(region, thread_task_queue.capacity, Thread_Task_Internal);

    for (u32 i = 0; i < thread_count; i++)
    {
        Thread_Attrib* ta = thread_attribs + i;
        ta->start_semaphore = &thread_task_queue.start_semaphore;
        ta->queue = &thread_task_queue;
        ta->id = i;
        thread_pool[i] = thread_create(ta, thread_loop, 0, NULL);
    }
}

void threads_destroy()
{
    const u32 thread_count = array_size(thread_pool);
    for (u32 i = 0; i < thread_count; i++)
    {
        thread_destroy(thread_pool[i]);
    }
}

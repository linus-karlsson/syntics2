#include "thread_queue.h"
#ifndef SY_UNIT_BUILD
#include "syntics_platform.h"
#include "logging.h"
#include "region_alloc.h"
#include <stdlib.h>
#endif

// TODO: Fibers and spin locks instead of semaphores
// global _Atomic u32 atomic_counter = 0;
// spin lock :
//      while(counter != value);

u32 global_thread_count = 0;

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
            platform_semaphore_wait_and_decrement(semaphore_counter->sempahore);
        }
        semaphore_counter->count = 0;
    }
}

void semaphore_counter_wait_and_free(Semaphore_Counter* semaphore_counter)
{
    semaphore_counter_wait(semaphore_counter);
    platform_semaphore_destroy(semaphore_counter->sempahore);
    free(semaphore_counter->sempahore);
}

// TODO: not use mutexes
// init semaphore with value one
// platform_semaphore_wait_and_decrement(...) as mutex lock
// the code
// platform_semaphore_increment(...) as unlock

void thread_task_push_(Thread_Task_Queue* task_queue, Thread_Task task,
                       Semaphore* semaphore)
{
    platform_semaphore_wait_and_decrement(&task_queue->mutex);

    assert(task_queue->size < task_queue->capacity);

    task_queue->tail %= task_queue->capacity;
    task_queue->tasks[task_queue->tail] =
        (Thread_Task_Internal){ .task = task, .sempahore = semaphore };

    // printf("Tail: %u\n", task_queue->tail);
    task_queue->tail++;
    task_queue->size++;

    platform_semaphore_increment(&task_queue->mutex);

    platform_semaphore_increment(&task_queue->start_semaphore);
}

void thread_tasks_push(Region_Alloc* region, Thread_Task_Queue* task_queue,
                       Thread_Task* tasks, u32 task_count,
                       Semaphore_Counter* semaphore_counter)
{
    if (semaphore_counter)
    {
        if (!semaphore_counter->sempahore && task_count)
        {
            semaphore_counter->sempahore =
                region ? region_calloc_struct(region, Semaphore)
                       : (Semaphore*)calloc(1, sizeof(Semaphore));
            *semaphore_counter->sempahore =
                platform_semaphore_create(0, task_count);
        }
        semaphore_counter->count = task_count;
    }
    for (u32 i = 0; i < task_count; i++)
    {
        thread_task_push_(task_queue, tasks[i], semaphore_counter->sempahore);
    }
}

internal Thread_Task_Internal thread_task_pop(Thread_Task_Queue* task_queue)
{
    platform_semaphore_wait_and_decrement(&task_queue->mutex);

    assert(task_queue->size > 0);

    task_queue->head %= task_queue->capacity;
    Thread_Task_Internal task = task_queue->tasks[task_queue->head];

    // printf("Head: %u\n", task_queue->head);
    task_queue->head++;
    task_queue->size--;

    platform_semaphore_increment(&task_queue->mutex);
    return task;
}

thread_return_value thread_loop(void* data)
{
    Thread_Attrib* attrib = (Thread_Attrib*)data;

    for (;;)
    {
        platform_semaphore_wait_and_decrement(attrib->start_semaphore);

        // printf("Thread %u start\n",attrib->id);

        Thread_Task_Internal task = thread_task_pop(attrib->queue);
        task.task.task_callback(task.task.data);

        // printf("Thread %u end\n",attrib->id);

        if (task.sempahore)
        {
            platform_semaphore_increment(task.sempahore);
        }
    }
}

void thread_init(Region_Alloc* region, u32 capacity, u32 thread_count,
                 Thread_Queue* queue)
{
    assert(!queue->pool);
    if (thread_count > 8)
    {
        thread_count = 8;
    }
    queue->pool = region_array_calloc(region, thread_count, Thread_Handle);
    queue->attribs = region_calloc(region, thread_count, Thread_Attrib);
    global_thread_count = thread_count;

    Semaphore start_semaphore = platform_semaphore_create(0, capacity);
    Semaphore mutex = platform_semaphore_create(1, capacity);
    queue->task_queue.start_semaphore = start_semaphore;
    queue->task_queue.mutex = mutex;
    queue->task_queue.capacity = capacity;
    queue->task_queue.tasks =
        region_calloc(region, queue->task_queue.capacity, Thread_Task_Internal);

    for (u32 i = 0; i < thread_count; i++)
    {
        Thread_Attrib* ta = queue->attribs + i;
        ta->start_semaphore = &queue->task_queue.start_semaphore;
        ta->queue = &queue->task_queue;
        ta->id = i;
        queue->pool[i] = platform_thread_create(ta, thread_loop, 0, NULL);
    }
}

void threads_destroy(Thread_Queue* queue)
{
    const u32 thread_count = region_array_size(queue->pool);
    for (u32 i = 0; i < thread_count; i++)
    {
        platform_thread_destroy(queue->pool[i]);
    }
}

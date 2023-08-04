typedef struct Thread_Task
{
    void (*task_callback)(void* data);
    void* data;
} Thread_Task;

typedef struct Thread_Task_Queue
{
    Mutex mutex;
    Semaphore start_semaphore;
    Semaphore end_semaphore;
    Thread_Task* tasks;
    u32 size;
    u32 _capacity;
} Thread_Task_Queue;

typedef struct Thread_Attrib
{
    Semaphore start_semaphore;
    Semaphore end_semaphore;
    Thread_Task_Queue* queue;
} Thread_Attrib;

// TODO: only one queue for the moment
#define MAX_THREADS 4
global Thread_Handle thread_pool[MAX_THREADS] = { 0 };
global Thread_Attrib thread_attribs[MAX_THREADS] = { 0 };
global Thread_Task_Queue thread_task_queue = { 0 };

Semaphore thread_task_push(void (*task_callback)(void* data), void* data)
{
    mutex_lock(&thread_task_queue.mutex);

    assert(thread_task_queue.size < thread_task_queue._capacity);
    thread_task_queue.tasks[thread_task_queue.size++] =
        (Thread_Task){ .task_callback = task_callback, .data = data };

    mutex_unlock(&thread_task_queue.mutex);

    semaphore_release(&thread_task_queue.start_semaphore);
    return thread_task_queue.end_semaphore;
}

Thread_Task thread_task_pop()
{
    mutex_lock(&thread_task_queue.mutex);

    assert(thread_task_queue.size > 0);
    Thread_Task task = thread_task_queue.tasks[--thread_task_queue.size];

    mutex_unlock(&thread_task_queue.mutex);
    return task;
}

thread_return_value thread_loop(void* data)
{
    Thread_Attrib* attrib = (Thread_Attrib*)data;

    for (;;)
    {
        semaphore_wait(&attrib->start_semaphore);

        Thread_Task task = thread_task_pop();
        task.task_callback(task.data);

        semaphore_release(&attrib->end_semaphore);
    }
}

void thread_init(Region_Alloc* region, u32 capacity)
{
    Semaphore start_semaphore = semaphore_create(0, sy_SIZE(thread_pool));
    Semaphore end_semaphore = semaphore_create(0, sy_SIZE(thread_pool));
    Mutex mutex = mutex_create();
    thread_task_queue.start_semaphore = start_semaphore;
    thread_task_queue.end_semaphore = end_semaphore;
    thread_task_queue.mutex = mutex;
    thread_task_queue._capacity = capacity;
    thread_task_queue.tasks =
        region_malloc(region, thread_task_queue._capacity, Thread_Task);

    for (u32 i = 0; i < MAX_THREADS; i++)
    {
        Thread_Attrib* ta = thread_attribs + i;
        ta->start_semaphore = start_semaphore;
        ta->end_semaphore = end_semaphore;
        ta->queue = &thread_task_queue;
        thread_pool[i] = thread_create(ta, thread_loop, 0, NULL);
    }
}

void threads_destroy()
{
    for (u32 i = 0; i < MAX_THREADS; i++)
    {
        thread_destroy(thread_pool[i]);
    }
}

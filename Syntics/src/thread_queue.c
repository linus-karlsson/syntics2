typedef struct Thread_Task
{
    void (*task_callback)(void* data);
    void* data;
} Thread_Task;

typedef struct Thread_Task_Queue
{
    HANDLE mutex;
    HANDLE start_semaphore;
    HANDLE end_semaphore;
    Thread_Task* tasks;
    u32 size;
    u32 _capacity;
} Thread_Task_Queue;

typedef struct Thread_Attrib
{
    HANDLE start_semaphore;
    HANDLE end_semaphore;
    Thread_Task_Queue* queue;
} Thread_Attrib;

// TODO: only one queue for the moment
#define MAX_THREADS 4
global HANDLE thread_pool[MAX_THREADS] = { 0 };
global Thread_Attrib thread_attribs[MAX_THREADS] = { 0 };
global Thread_Task_Queue thread_task_queue = { 0 };

HANDLE thread_task_push(void (*task_callback)(void* data), void* data)
{
    WaitForSingleObject(thread_task_queue.mutex, INFINITE);

    assert(thread_task_queue.size < thread_task_queue._capacity);
    thread_task_queue.tasks[thread_task_queue.size++] =
        (Thread_Task){ .task_callback = task_callback, .data = data };

    ReleaseMutex(thread_task_queue.mutex);

    ReleaseSemaphore(thread_task_queue.start_semaphore, 1, 0);
    return thread_task_queue.end_semaphore;
}

Thread_Task thread_task_pop()
{
    WaitForSingleObject(thread_task_queue.mutex, INFINITE);

    assert(thread_task_queue.size > 0);
    Thread_Task task = thread_task_queue.tasks[--thread_task_queue.size];

    ReleaseMutex(thread_task_queue.mutex);
    return task;
}

unsigned long thread_loop(void* data)
{
    Thread_Attrib* attrib = (Thread_Attrib*)data;

    for (;;)
    {
        WaitForSingleObject(attrib->start_semaphore, INFINITE);

        Thread_Task task = thread_task_pop();
        task.task_callback(task.data);

        ReleaseSemaphore(attrib->end_semaphore, 1, 0);
    }
}

void thread_init(Region_Alloc* region, u32 capacity)
{
    HANDLE start_semaphore = CreateSemaphore(NULL, 0, sy_SIZE(thread_pool), NULL);
    HANDLE end_semaphore = CreateSemaphore(NULL, 0, sy_SIZE(thread_pool), NULL);
    HANDLE mutex = CreateMutex(NULL, false, NULL);
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

void thread_destroy()
{
    for (u32 i = 0; i < MAX_THREADS; i++)
    {
        CloseHandle(thread_pool[i]);
    }
}


#include <cmath>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sched.h> // for CPU affinity functions

#include "../includes/ringbuf.h"

void usage()
{
    std::cout << "Usage: testbench -n <int> -l <int>" << std::endl;
    std::cout << "    -n: Number of nodes." << std::endl;
    std::cout << "    -l: Number of entry in buffer." << std::endl;
}

class ThreadArgs
{
public:
    int m_thread_id;
    RingBuffer *m_ring_buf;

    int total_op_num = 0;
    int valid_op_num = 0;
    double elapsed = 0.0;
};

class BufferData
{
public:
    int data;
};

#define OPERATION 100000

pthread_barrier_t global_barrier;

void *thread_function(void *args)
{
    pthread_barrier_wait(&global_barrier);

    ThreadArgs *thread_args = (ThreadArgs *)args;
    int thread_id = thread_args->m_thread_id;
    RingBuffer *ring_buffer = thread_args->m_ring_buf;

    struct timespec start, end;

    // enqueue
    for (int i = 0; i < 3 * OPERATION; i++)
    {
        // dequeue
        void *data_ptr;
        while (true)
        {
            int res = dequeue_ringbuf(ring_buffer, &data_ptr);
            if (i >= OPERATION && i <= 2 * OPERATION)
            {
                thread_args->total_op_num += 1;
            }
            if (res == 0)
            {
                if (i >= OPERATION && i <= 2 * OPERATION)
                {
                    thread_args->valid_op_num += 1;
                }
                break;
            }
        }
        BufferData *data = (BufferData *)data_ptr;
        // data->data = thread_id;

        while (true)
        {
            int res = enqueue_ringbuf(ring_buffer, data);
            if (i >= OPERATION && i <= 2 * OPERATION)
            {
                thread_args->total_op_num += 1;
            }
            if (res == 0)
            {
                if (i >= OPERATION && i <= 2 * OPERATION)
                {
                    thread_args->valid_op_num += 1;
                }
                break;
            }
        }

        if (i == OPERATION)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
        }
        else if (i == 2 * OPERATION)
        {
            clock_gettime(CLOCK_MONOTONIC, &end);
        }
    }

    thread_args->elapsed = (end.tv_sec - start.tv_sec) +
                           (end.tv_nsec - start.tv_nsec) / 1e9;
    std::cout << "total_op=" << thread_args->total_op_num
              << "\tvalid_op=" << thread_args->valid_op_num
              << "\ttime=" << thread_args->elapsed << std::endl;

    return NULL;
}

int main(int argc, char *argv[])
{
    // Check input arguments.
    if (argc != 5)
    {
        usage();
        exit(EXIT_FAILURE);
    }
    char opt;
    int thread_num, entry_num;
    while ((opt = getopt(argc, argv, "n:l:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            thread_num = std::stoi(optarg);
            break;
        case 'l':
            entry_num = std::stoi(optarg);
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "Require " << thread_num << " threads." << std::endl;

    // Create ring buffer.
    RingBuffer ring_buffer = RingBuffer(entry_num);
    init_ringbuf(&ring_buffer);
    for (int i = 0; i < entry_num - 1; i++)
    {
        BufferData *data = (BufferData *)malloc(sizeof(BufferData));
        data->data = -1;
        enqueue_ringbuf(&ring_buffer, data);
    }
    std::cout << "Create shared ring buffer." << std::endl;

    pthread_t threads[thread_num];
    ThreadArgs threadIds[thread_num]; // To store thread IDs

    // Initialize barrier
    pthread_barrier_init(&global_barrier, NULL, thread_num);

    // Core mapping array
    int cpu_num = 2;
    int core_per_cpu = sysconf(_SC_NPROCESSORS_ONLN) / cpu_num;

    // Create threads
    for (int i = 0; i < thread_num; i++)
    {
        threadIds[i].m_ring_buf = &ring_buffer;
        threadIds[i].m_thread_id = i;
        if (pthread_create(
                &threads[i], NULL, thread_function, &threadIds[i]) != 0)
        {
            std::cerr << "Error: Unable to create thread " << i << "\n";
            return 1;
        }

        // Map each thread to a specific core
        int coreId = (i / cpu_num) + (i % cpu_num) * core_per_cpu;

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);        // Clear the CPU set
        CPU_SET(coreId, &cpuset); // Add the specified core to the set

        // Set thread affinity
        int result =
            pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
        if (result != 0)
        {
            std::cerr << "Error setting thread affinity: "
                      << strerror(result) << "\n";
        }
        else
        {
            std::cout << "Thread " << i << " mapped to core " << coreId << "\n";
        }
    }

    double total_speed = 0;
    double valid_speed = 0;

    // Wait for all threads to finish
    for (int i = 0; i < thread_num; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            std::cerr << "Error: Unable to join thread " << i << "\n";
        }
        total_speed += threadIds[i].total_op_num / threadIds[i].elapsed;
        valid_speed += threadIds[i].valid_op_num / threadIds[i].elapsed;
    }

    std::cout << "All threads have completed.\n";
    std::cout << "Total operation speed: " << total_speed << " op/s.\n";
    std::cout << "Valid operation speed: " << valid_speed << " op/s.\n";
    return 0;
}


#include <cmath>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sched.h> // for CPU affinity functions

#include "../includes/ringbuf.h"

void usage()
{
    std::cout << "Usage: testbench -n <int> [-l <int>] [-o <int>] [-m <str>]" << std::endl;
    std::cout << "    -n: Number of nodes." << std::endl;
    std::cout << "    -l: Number of entry in buffer." << std::endl;
    std::cout << "    -o: Number of operations under test." << std::endl;
    std::cout << "    -m: Core mapping method. Options: numa1, numa2, numa1-phy, numa2-phy" << std::endl;
}

class ThreadArgs
{
public:
    int m_thread_id;
    int m_operation_num;
    RingBuffer *m_ring_buf;

    int valid_op_num = 0;
    double elapsed = 0.0;
};

class BufferData
{
public:
    int data;
};

pthread_barrier_t global_barrier;

void *thread_function(void *args)
{
    pthread_barrier_wait(&global_barrier);

    ThreadArgs *thread_args = (ThreadArgs *)args;
    int thread_id = thread_args->m_thread_id;
    int operation_num = thread_args->m_operation_num;
    RingBuffer *ring_buffer = thread_args->m_ring_buf;

    struct timespec start, end;

    // enqueue
    for (int i = 0; i < 3 * operation_num; i++)
    {
        // dequeue
        void *data_ptr;
        int res = 1;
        while (res)
        {
            res = dequeue_ringbuf(ring_buffer, &data_ptr);
        }

        BufferData *data = (BufferData *)data_ptr;
        data->data = thread_id;

        // enqueue
        res = 1;
        while (res)
        {
            res = enqueue_ringbuf(ring_buffer, data_ptr);
        }

        // Increase counter.
        if (i > operation_num && i <= 2 * operation_num)
        {
            thread_args->valid_op_num += 2;
        }

        if (i == operation_num)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
        }
        else if (i == 2 * operation_num)
        {
            clock_gettime(CLOCK_MONOTONIC, &end);
        }
    }

    thread_args->elapsed = (end.tv_sec - start.tv_sec) +
                           (end.tv_nsec - start.tv_nsec) / 1e9;
    return NULL;
}

int core_mapping(int thread_id, char *rule)
{
    int core_id = thread_id;
    // Only 1 Numa node, logical cores.
    if (strcmp(rule, "numa1") == 0)
    {
        core_id = thread_id;
    }
    // 2 Numa nodes, logical cores.
    else if (strcmp(rule, "numa2") == 0)
    {
        int core_per_cpu = sysconf(_SC_NPROCESSORS_ONLN) / 2;
        core_id = thread_id / 2 + (thread_id % 2) * core_per_cpu;
    }
    // Only 1 Numa node, only physical core.
    else if (strcmp(rule, "numa1-phy") == 0)
    {
        core_id = thread_id * 2;
    }
    // 2 Numa nodes, only physical core.
    else if (strcmp(rule, "numa2-phy") == 0)
    {
        int core_per_cpu = sysconf(_SC_NPROCESSORS_ONLN) / 2;
        core_id = (thread_id / 2) * 2 + (thread_id % 2) * core_per_cpu;
    }
    else {
        std::cerr << "Unknown core mapping rule." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (core_id > sysconf(_SC_NPROCESSORS_ONLN)) {
        std::cerr << "Error: Unexist core " << core_id
                  << " (for thread " << thread_id << ")." << std::endl;
        exit(EXIT_FAILURE);
    }

    return core_id;
}

int main(int argc, char *argv[])
{
    // Check input arguments.
    if (argc != 3 && argc != 5 && argc != 7 && argc != 9)
    {
        usage();
        exit(EXIT_FAILURE);
    }
    int opt;
    int thread_num, entry_num = 256, operation_num = 100000;
    char core_mapping_rule[20] = "numa1";
    while ((opt = getopt(argc, argv, "n:l:o:m:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            thread_num = std::stoi(optarg);
            break;
        case 'l':
            entry_num = std::stoi(optarg);
            break;
        case 'o':
            operation_num = std::stoi(optarg);
            break;
        case 'm':
            strcpy(core_mapping_rule, optarg);
            break;
        default:
            usage();
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "Arguments:"
              << " -n " << thread_num
              << " -l " << entry_num
              << " -o " << operation_num
              << " -m " << core_mapping_rule << std::endl;

    // Core mapping.
    int core_id[thread_num];
    std::cout << "Core mapping:";
    for (int i = 0; i < thread_num; i ++) {
        core_id[i] = core_mapping(i, core_mapping_rule);
        std::cout << " " << core_id[i];
    }
    std::cout << std::endl;

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

    // Create threads
    for (int i = 0; i < thread_num; i++)
    {
        threadIds[i].m_ring_buf = &ring_buffer;
        threadIds[i].m_thread_id = i;
        threadIds[i].m_operation_num = operation_num;
        if (pthread_create(
                &threads[i], NULL, thread_function, &threadIds[i]) != 0)
        {
            std::cerr << "Error: Unable to create thread " << i << "\n";
            return 1;
        }

        // Map each thread to a specific core
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);        // Clear the CPU set
        CPU_SET(core_id[i], &cpuset); // Add the specified core to the set

        // Set thread affinity
        int result =
            pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
        if (result != 0)
        {
            std::cerr << "Error setting thread affinity: "
                      << strerror(result) << "\n";
        }
    }

    double valid_speed = 0;

    // Wait for all threads to finish
    int valid_op_num[thread_num] = {0};
    double elapsed[thread_num] = {0.0};
    for (int i = 0; i < thread_num; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            std::cerr << "Error: Unable to join thread " << i << "\n";
        }

        valid_op_num[i] = threadIds[i].valid_op_num;
        elapsed[i] = threadIds[i].elapsed;
        valid_speed += threadIds[i].valid_op_num / threadIds[i].elapsed;
    }

    std::cout << "All threads have completed.\n";
    std::cout << "Valid Op:";
    for (int i = 0; i < thread_num; i++)
    {
        std::cout << " " << threadIds[i].valid_op_num;
    }
    std::cout << std::endl;
    std::cout << "Elapsed :";
    for (int i = 0; i < thread_num; i++)
    {
        std::cout << " " << threadIds[i].elapsed;
    }
    std::cout << std::endl;
    std::cout << "Valid operation speed: " << valid_speed << " op/s.\n";
    return 0;
}

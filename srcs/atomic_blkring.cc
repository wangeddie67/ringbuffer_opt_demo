
#include "../includes/ringbuf.h"
#include <atomic>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <iostream>
#include <cstring>

int init_ringbuf(RingBuffer *ringbuffer)
{
    ringbuffer->m_enqueue_mutex = 0;
    ringbuffer->m_dequeue_mutex = 0;
    ringbuffer->m_head = 0;
    ringbuffer->m_tail = 0;
    ringbuffer->m_entries =
        (BufferEntry *)calloc(ringbuffer->m_size, sizeof(BufferEntry));
    memset(ringbuffer->m_entries, 0, ringbuffer->m_size);

    return 0;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_head, 1, __ATOMIC_RELAXED);
    int enq_ptr = sn % ring_buffer->m_size;

    while (true)
    {
        void* entry_ptr = __atomic_load_n(&ring_buffer->m_entries[enq_ptr].m_ptr, __ATOMIC_ACQUIRE);
        // Ringbuffer is full. Do nothing.
        if (entry_ptr != NULL)
        {
        }
        else
        {
            __atomic_store_n(&ring_buffer->m_entries[enq_ptr].m_ptr, entry, __ATOMIC_RELEASE);
            break;
        }
    }

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_tail, 1, __ATOMIC_RELAXED);
    int deq_ptr = sn % ring_buffer->m_size;

    while (true)
    {
        void* entry_ptr = __atomic_load_n(&ring_buffer->m_entries[deq_ptr].m_ptr, __ATOMIC_ACQUIRE);
        // Ringbuffer is empty. Do nothing.
        if (entry_ptr == NULL)
        {
        }
        else
        {
            *entry = entry_ptr;
            __atomic_store_n(&ring_buffer->m_entries[deq_ptr].m_ptr, 0, __ATOMIC_RELEASE);
            break;
        }
    }

    return 0;
}

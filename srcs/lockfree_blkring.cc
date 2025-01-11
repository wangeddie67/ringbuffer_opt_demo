
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
    for (int i = 0; i < ringbuffer->m_size; i++)
    {
        ringbuffer->m_entries[i].m_sn = i;
    }

    return 0;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_head, 1, __ATOMIC_RELAXED);
    int enq_ptr = sn % ring_buffer->m_size;

    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_sn = ring_buffer->m_entries[enq_ptr].m_sn;
        entry_ptr = ring_buffer->m_entries[enq_ptr].m_ptr;
        // Ringbuffer is full. Do nothing.
    } while (entry_sn != sn || entry_ptr != NULL);

    ring_buffer->m_entries[enq_ptr].m_ptr = entry;

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_tail, 1, __ATOMIC_RELAXED);
    int deq_ptr = sn % ring_buffer->m_size;

    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_sn = ring_buffer->m_entries[deq_ptr].m_sn;
        entry_ptr = ring_buffer->m_entries[deq_ptr].m_ptr;
    } while (entry_ptr == NULL || entry_sn != sn);

    *entry = entry_ptr;
    ring_buffer->m_entries[deq_ptr].m_ptr = 0;
    ring_buffer->m_entries[deq_ptr].m_sn = sn + ring_buffer->m_size;

    return 0;
}


#include "../includes/ringbuf.h"
#include <atomic>
#include <sys/mman.h>
#include <cstring>

// Lock function
void lock(int *mutex)
{
    while (__sync_val_compare_and_swap(mutex, 0, 1) != 0)
    {
    }
}

// Unlock function
void unlock(int *mutex)
{
    __sync_val_compare_and_swap(mutex, 1, 0);
}

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
    lock(&ring_buffer->m_enqueue_mutex);

    // Ringbuffer is full. Do nothing.
    bool full = true;
    do
    {
        full = (ring_buffer->m_head == ring_buffer->m_tail - 1) ||
               (ring_buffer->m_tail == 0 &&
                ring_buffer->m_head == ring_buffer->m_size - 1);
    } while (full);

    // Enqueue entry.
    ring_buffer->m_entries[ring_buffer->m_head].m_ptr = entry;
    // Increase head pointer.
    ring_buffer->m_head = (ring_buffer->m_head + 1) % ring_buffer->m_size;

    unlock(&ring_buffer->m_enqueue_mutex);

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    lock(&ring_buffer->m_dequeue_mutex);

    // Ringbuffer is empty. Do nothing.
    bool empty = true;
    do
    {
        empty = ring_buffer->m_head == ring_buffer->m_tail;
    } while (empty);

    // Dequeue entry.
    *entry = ring_buffer->m_entries[ring_buffer->m_tail].m_ptr;
    // Increase tail pointer.
    ring_buffer->m_tail = (ring_buffer->m_tail + 1) % ring_buffer->m_size;

    unlock(&ring_buffer->m_dequeue_mutex);

    return 0;
}


#include "../includes/ringbuf.h"
#include <atomic>
#include <sys/mman.h>
#include <cstring>

// Lock function
int lock(int *mutex)
{
    return __sync_val_compare_and_swap(mutex, 0, 1);
}

// Unlock function
int unlock(int *mutex)
{
    __sync_val_compare_and_swap(mutex, 1, 0);
    return 0;
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
    int res = 0;

    // Lock mutex.
    res = lock(&ring_buffer->m_enqueue_mutex);
    if (res != 0) {
        return res;
    }

    // Ringbuffer is full. Do nothing.
    if ((ring_buffer->m_head == ring_buffer->m_tail - 1) ||
        (ring_buffer->m_tail == 0 &&
            ring_buffer->m_head == ring_buffer->m_size - 1))
    {
        res = 1;
    }
    else
    {
        ring_buffer->m_entries[ring_buffer->m_head].m_ptr = entry;
        ring_buffer->m_head =
            (ring_buffer->m_head + 1) % ring_buffer->m_size;
        res = 0;
    }
    unlock(&ring_buffer->m_enqueue_mutex);

    return res;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    int res = 0;

    // Lock mutex.
    res = lock(&ring_buffer->m_dequeue_mutex);
    if (res != 0) {
        return res;
    }

    // Ringbuffer is empty. Do nothing.
    if (ring_buffer->m_head == ring_buffer->m_tail)
    {
        res = 1;
    }
    else
    {
        *entry = ring_buffer->m_entries[ring_buffer->m_tail].m_ptr;
        ring_buffer->m_tail =
            (ring_buffer->m_tail + 1) % ring_buffer->m_size;
        res = 0;
    }
    unlock(&ring_buffer->m_dequeue_mutex);

    return res;
}

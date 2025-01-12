
#include <atomic>
#include <cstring>
#include <sys/mman.h>
#include <stdlib.h>

// Entry in ring buffer
class BufferEntry
{
public:
    void *mp_ptr;    // Pointer to data entry.

public:
    BufferEntry()
        : mp_ptr(NULL)
    {
    }
};

// Data structure for ring buffer
class RingBuffer
{
public:
    int m_enqueue_mutex;
    int m_dequeue_mutex;
    unsigned int m_size;

    unsigned int m_head;
    unsigned int m_tail;

    BufferEntry *mp_entries;

public:
    RingBuffer(int entry_num)
        : m_enqueue_mutex(0),
          m_dequeue_mutex(0),
          m_size(entry_num),
          m_head(0),
          m_tail(0),
          mp_entries(NULL)
    {
        mp_entries = (BufferEntry *)calloc(m_size, sizeof(BufferEntry));
        memset(mp_entries, 0, sizeof(BufferEntry) * m_size);
    }
};

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

RingBuffer* create_ringbuf(int entry_num)
{
    RingBuffer* ring_buf = new RingBuffer(entry_num);
    return ring_buf;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    lock(&ring_buffer->m_enqueue_mutex);

    // Ringbuffer is full. Do nothing.
    while (ring_buffer->m_head + ring_buffer->m_size == ring_buffer->m_tail);

    // Enqueue entry.
    int enq_ptr = ring_buffer->m_tail % ring_buffer->m_size;
    ring_buffer->mp_entries[enq_ptr].mp_ptr = entry;
    // Increase tail pointer.
    ring_buffer->m_tail = ring_buffer->m_tail + 1;

    unlock(&ring_buffer->m_enqueue_mutex);

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    lock(&ring_buffer->m_dequeue_mutex);

    // Ringbuffer is empty. Do nothing.
    while (ring_buffer->m_head == ring_buffer->m_tail);

    // Dequeue entry.
    int deq_ptr = ring_buffer->m_head % ring_buffer->m_size;
    *entry = ring_buffer->mp_entries[deq_ptr].mp_ptr;
    ring_buffer->mp_entries[deq_ptr].mp_ptr = NULL;
    // Increase head pointer.
    ring_buffer->m_head = ring_buffer->m_head + 1;

    unlock(&ring_buffer->m_dequeue_mutex);

    return 0;
}

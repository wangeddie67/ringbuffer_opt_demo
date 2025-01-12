
#include <atomic>
#include <cstring>
#include <sys/mman.h>
#include <stdlib.h>

// Entry in ring buffer
class BufferEntry
{
public:
    unsigned int m_sn;  // Sequential number.
    void *mp_ptr;       // Pointer to data entry.

public:
    BufferEntry()
        : m_sn(0),
          mp_ptr(NULL)
    {
    }
};

// Data structure for ring buffer
class RingBuffer
{
public:
    unsigned int m_size;

    unsigned int m_head;
    unsigned int m_tail;

    BufferEntry *mp_entries;

public:
    RingBuffer(int entry_num)
        : m_size(entry_num),
          m_head(0),
          m_tail(0),
          mp_entries(NULL)
    {
        mp_entries = (BufferEntry *)calloc(m_size, sizeof(BufferEntry));
        memset(mp_entries, 0, sizeof(BufferEntry) * m_size);

        for (int i = 0; i < m_size; i++)
        {
            mp_entries[i].m_sn = i;
        }
    }
};

RingBuffer* create_ringbuf(int entry_num)
{
    RingBuffer* ring_buf = new RingBuffer(entry_num);
    return ring_buf;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_head, 1, __ATOMIC_RELAXED);
    int enq_ptr = sn % ring_buffer->m_size;

    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_sn = ring_buffer->mp_entries[enq_ptr].m_sn;
        entry_ptr = ring_buffer->mp_entries[enq_ptr].mp_ptr;
    } while (entry_sn != sn || entry_ptr != NULL);

    ring_buffer->mp_entries[enq_ptr].mp_ptr = entry;

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
        entry_sn = ring_buffer->mp_entries[deq_ptr].m_sn;
        entry_ptr = ring_buffer->mp_entries[deq_ptr].mp_ptr;
    } while (entry_ptr == NULL || entry_sn != sn);

    *entry = entry_ptr;
    ring_buffer->mp_entries[deq_ptr].mp_ptr = 0;
    ring_buffer->mp_entries[deq_ptr].m_sn = sn + ring_buffer->m_size;

    return 0;
}

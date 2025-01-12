
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
    alignas(64) unsigned int m_size;

    alignas(64) unsigned int m_head;
    alignas(64) unsigned int m_tail;

    alignas(64) BufferEntry *mp_entries;

public:
    RingBuffer(int entry_num)
        : m_size(entry_num),
          m_head(0),
          m_tail(0),
          mp_entries(NULL)
    {
        mp_entries = (BufferEntry *)calloc(m_size * 4, sizeof(BufferEntry));
        memset(mp_entries, 0, sizeof(BufferEntry) * m_size * 4);

        for (int i = 0; i < m_size; i++)
        {
            mp_entries[i * 4].m_sn = i;
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
    int sn = __atomic_fetch_add(&ring_buffer->m_tail, 1, __ATOMIC_RELAXED);
    int enq_ptr = (sn % ring_buffer->m_size) * 4;

    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_sn = __atomic_load_n(
            &ring_buffer->mp_entries[enq_ptr].m_sn, __ATOMIC_RELAXED);
        entry_ptr = __atomic_load_n(
            &ring_buffer->mp_entries[enq_ptr].mp_ptr, __ATOMIC_RELAXED);
    } while (entry_sn != sn || entry_ptr != NULL);

    __atomic_store_n(&ring_buffer->mp_entries[enq_ptr].mp_ptr, entry,
                     __ATOMIC_RELEASE);

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    int sn = __atomic_fetch_add(&ring_buffer->m_head, 1, __ATOMIC_RELAXED);
    int deq_ptr = (sn % ring_buffer->m_size) * 4;

    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_sn = __atomic_load_n(
            &ring_buffer->mp_entries[deq_ptr].m_sn, __ATOMIC_RELAXED);
        entry_ptr = __atomic_load_n(
            &ring_buffer->mp_entries[deq_ptr].mp_ptr, __ATOMIC_ACQUIRE);
    } while (entry_ptr == NULL || entry_sn != sn);

    *entry = entry_ptr;
    __atomic_store_n(&ring_buffer->mp_entries[deq_ptr].mp_ptr, 0,
                     __ATOMIC_RELAXED);
    __atomic_store_n(&ring_buffer->mp_entries[deq_ptr].m_sn,
                     sn + ring_buffer->m_size, __ATOMIC_RELEASE);

    return 0;
}


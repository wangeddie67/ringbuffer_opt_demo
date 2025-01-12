
#include <atomic>
#include <cstring>
#include <sys/mman.h>
#include <stdlib.h>

// Entry in ring buffer
class BufferEntry
{
public:
    void *mp_ptr; // Pointer to data entry.

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
    alignas(64) unsigned int m_size;

    // Producer block, allocated for enqueue, cover empty items.
    alignas(64) unsigned int m_prod_head; // read pointer
    alignas(64) unsigned int m_prod_tail; // write pointer

    // Consumer block, allocated for dequeue, cover not-empty items.
    alignas(64) unsigned int m_cons_head; // write pointer
    alignas(64) unsigned int m_cons_tail; // read pointer

    alignas(64) BufferEntry *mp_entries;

public:
    RingBuffer(int entry_num)
        : m_size(entry_num),
          m_prod_head(0),
          m_prod_tail(0),
          m_cons_head(0),
          m_cons_tail(0),
          mp_entries(NULL)
    {
        mp_entries = (BufferEntry *)calloc(m_size * 8, sizeof(BufferEntry));
        memset(mp_entries, 0, sizeof(BufferEntry) * m_size * 8);
    }
};

RingBuffer *create_ringbuf(int entry_num)
{
    RingBuffer *ring_buf = new RingBuffer(entry_num);
    return ring_buf;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    // Step 1: acquire slots
    unsigned int tail = __atomic_load_n(
        &ring_buffer->m_prod_tail, __ATOMIC_RELAXED);
    do
    {
        unsigned int head = __atomic_load_n(
            &ring_buffer->m_prod_head, __ATOMIC_ACQUIRE);
        if (ring_buffer->m_size + head == tail)
        {
            return 1;
        }
    } while (!__atomic_compare_exchange_n(&ring_buffer->m_prod_tail,
                                          &tail, // Updated on failure
                                          tail + 1,
                                          /*weak=*/true,
                                          __ATOMIC_RELAXED,
                                          __ATOMIC_RELAXED));

    // Step 2: Write slots
    int enq_ptr = (tail % ring_buffer->m_size) * 8;
    void *entry_ptr;
    do
    {
        entry_ptr = __atomic_load_n(
            &ring_buffer->mp_entries[enq_ptr].mp_ptr, __ATOMIC_RELAXED);
    } while (entry_ptr != NULL);

    __atomic_store_n(&ring_buffer->mp_entries[enq_ptr].mp_ptr, entry,
                     __ATOMIC_RELEASE);

    // Finally make all released slots available for new acquisitions
    __atomic_fetch_add(&ring_buffer->m_cons_tail, 1, __ATOMIC_RELEASE);
    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    // Step 1: acquire slots
    unsigned int tail = __atomic_load_n(
        &ring_buffer->m_cons_head, __ATOMIC_RELAXED);
    do
    {
        unsigned int head = __atomic_load_n(
            &ring_buffer->m_cons_tail, __ATOMIC_ACQUIRE);
        if (head == tail)
        {
            return 1;
        }
    } while (!__atomic_compare_exchange_n(&ring_buffer->m_cons_head,
                                          &tail, // Updated on failure
                                          tail + 1,
                                          /*weak=*/true,
                                          __ATOMIC_RELAXED,
                                          __ATOMIC_RELAXED));

    // Step 2: Write slots (write NIL for dequeue)
    int deq_ptr = (tail % ring_buffer->m_size) * 8;
    unsigned int entry_sn;
    void *entry_ptr;
    do
    {
        entry_ptr = __atomic_load_n(
            &ring_buffer->mp_entries[deq_ptr].mp_ptr, __ATOMIC_ACQUIRE);
    } while (entry_ptr == NULL);

    *entry = entry_ptr;
    __atomic_store_n(&ring_buffer->mp_entries[deq_ptr].mp_ptr, 0,
                     __ATOMIC_RELAXED);

    // Finally make all released slots available for new acquisitions
    __atomic_fetch_add(&ring_buffer->m_prod_head, 1, __ATOMIC_RELEASE);
    return 0;
}

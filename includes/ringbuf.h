
#include <cstdlib>

class BufferEntry
{
public:
    void *m_ptr;

public:
    BufferEntry()
        : m_ptr(NULL)
    {
    }
};

class RingBuffer
{
public:
    unsigned int m_size;

    unsigned int m_head;
    unsigned int m_tail;

    BufferEntry *m_entries;

public:
    RingBuffer(int entry_num)
        : m_size(entry_num),
          m_head(0),
          m_tail(0),
          m_entries(NULL)
    {
    }
};

RingBuffer* create_ringbuf(int entry_num);
int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry);
int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry);

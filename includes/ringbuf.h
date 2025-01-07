
#include <cstdlib>

class BufferEntry
{
public:
    unsigned int m_sn;
    void *m_ptr;

public:
    BufferEntry()
        : m_sn(0),
          m_ptr(NULL)
    {
    }
};

class RingBuffer
{
public:
    int m_enqueue_mutex;
    int m_dequeue_mutex;
    unsigned int m_size;

    unsigned int m_head;
    unsigned int m_tail;

    BufferEntry *m_entries;

public:
    RingBuffer(int entry_num)
        : m_enqueue_mutex(0),
          m_dequeue_mutex(0),
          m_size(entry_num),
          m_head(0),
          m_tail(0),
          m_entries(NULL)
    {
    }
};

int init_ringbuf(RingBuffer *ringbuffer);
int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry);
int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry);

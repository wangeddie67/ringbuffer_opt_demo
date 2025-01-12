
#include <cstdlib>
#include "../progress64/include/p64_buckring.h"

#define RingBuffer p64_buckring_t

RingBuffer* create_ringbuf(int entry_num)
{
    RingBuffer* ring_buf = p64_buckring_alloc(entry_num, 0);
    return ring_buf;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    void* enq_ptr[] = {entry};
    int res = p64_buckring_enqueue(ring_buffer, enq_ptr, 1);

    return res ? 0 : 1;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    void* deq_ptr[] = {NULL};
	unsigned int index;
    int res = p64_buckring_dequeue(ring_buffer, deq_ptr, 1, &index);
    *entry = deq_ptr[0];

    return res ? 0 : 1;
}

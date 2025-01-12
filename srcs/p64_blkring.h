
#include "../progress64/include/p64_blkring.h"

#define RingBuffer p64_blkring_t

RingBuffer* create_ringbuf(int entry_num)
{
    RingBuffer* ring_buf = p64_blkring_alloc(entry_num);
    return ring_buf;
}

int enqueue_ringbuf(RingBuffer *ring_buffer, void *entry)
{
    void* enq_ptr[] = {entry};
    p64_blkring_enqueue(ring_buffer, enq_ptr, 1);

    return 0;
}

int dequeue_ringbuf(RingBuffer *ring_buffer, void **entry)
{
    void* deq_ptr[] = {NULL};
	unsigned int index;
    p64_blkring_dequeue(ring_buffer, deq_ptr, 1, &index);
    *entry = deq_ptr[0];

    return 0;
}

#ifndef CUT_RINGBUFFER_H
#define CUT_RINGBUFFER_H

#include <stdlib.h>


typedef struct RingBuffer
{
    size_t size_of_element;
    size_t max_queue_length;
    size_t current_index;
    size_t current_queue_length;
    char data_pointer[];
} RingBuffer;

/*
 * returns 1 if queue is full
 */
static inline int ring_buffer_is_full(RingBuffer* ring_buffer) {
    return ring_buffer->current_queue_length == ring_buffer->max_queue_length;
}

/*
 * returns 1 if queue is empty
 */
static inline int ring_buffer_is_empty(RingBuffer* ring_buffer) {
    return ring_buffer->current_queue_length == (size_t) 0;
}

/*
 * remove from buffer and copy it to given pointer
 * ptrToNewElement - pointer to memory where element from queue will be written
 * return 0 if failed 1 if succeeded
 */
int ring_buffer_pop(RingBuffer* ring_buffer, char* new_element);

/*
 * copy from pointer memoery of queue elementSize to queue
 * @param queue pointer to queue
 * @param ptrForReturnedElement pointer to element which will be copied to queue
 * @return 0 if failed
 */
int ring_buffer_push(RingBuffer* ring_buffer, const char* return_buffer);

/*
 * Works like ring_buffer_push but insted of copy returns pointer to element
 * @param ring_buffer pointer to queue
 * @return returns pointer to new "allocated" space in queue.
 */
char* ring_buffer_push_pointer(RingBuffer* ring_buffer);

/*
 * initializing queue with elements of given size and given length
 * @param ring_buffer pointer to queue
 * @param size_of_element size of elements in bytes
 * @param buffer_length max queue length
 */
RingBuffer* ring_buffer_create(size_t size_of_element, size_t buffer_length);

/*
 * freeing memory of queue
 * @param ring_buffer pointer to queue
 */
void ring_buffer_destroy(RingBuffer* ring_buffer);

#endif //CUT_RINGBUFFER_H

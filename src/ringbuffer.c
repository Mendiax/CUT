#include <ringbuffer.h>
#include <stdlib.h>
#include <string.h>

static char* ring_buffer_get_element_pointer(RingBuffer* ring_buffer, size_t index);
static void ring_buffer_inc_index(RingBuffer* queue);

static char* ring_buffer_get_element_pointer(RingBuffer* ring_buffer, size_t index) {
    return ring_buffer->data_pointer + ring_buffer->size_of_element * index;
}

static void ring_buffer_inc_index(RingBuffer* queue) {
    queue->current_index++;
    queue->current_index = queue->current_index % queue->max_queue_length;
}

int ring_buffer_pop(RingBuffer* ring_buffer, char* new_element) {
    if (ring_buffer_is_empty(ring_buffer)){
        return 0;
    }
    memcpy(new_element, ring_buffer_get_element_pointer(ring_buffer, ring_buffer->current_index), ring_buffer->size_of_element);
    ring_buffer_inc_index(ring_buffer);
    ring_buffer->current_queue_length--;
    return 1;
}

int ring_buffer_push(RingBuffer* ring_buffer, const char* element){
    if (ring_buffer_is_full(ring_buffer)) {
        return -1;
    }
    if (!ring_buffer->current_queue_length) {
        memcpy(ring_buffer_get_element_pointer(ring_buffer, ring_buffer->current_index), element, ring_buffer->size_of_element);
    } else {
        size_t nextFree = (ring_buffer->current_index + ring_buffer->current_queue_length) % ring_buffer->max_queue_length;
        memcpy(ring_buffer_get_element_pointer(ring_buffer, nextFree), element, ring_buffer->size_of_element);
    }
    ring_buffer->current_queue_length++;
    return 0;
}

char* ring_buffer_push_pointer(RingBuffer* ring_buffer) {
    if (ring_buffer_is_full(ring_buffer)) {
        return 0;
    }
    char* free_space;
    if (!ring_buffer->current_queue_length) {
        free_space = ring_buffer_get_element_pointer(ring_buffer, ring_buffer->current_index);
    } else {
        size_t next_free = (ring_buffer->current_index + ring_buffer->current_queue_length) % ring_buffer->max_queue_length;
        free_space = ring_buffer_get_element_pointer(ring_buffer, next_free);
    }
    ring_buffer->current_queue_length++;
    return free_space;
}

RingBuffer* ring_buffer_create(size_t size_of_element, size_t buffer_length) {
    RingBuffer* ring_buffer = malloc(sizeof(RingBuffer) + size_of_element *  buffer_length);
    ring_buffer->max_queue_length = buffer_length;
    ring_buffer->size_of_element = size_of_element;
    ring_buffer->current_queue_length = 0;
    ring_buffer->current_index = 0;
    return ring_buffer;
}

void ring_buffer_destroy(RingBuffer* ring_buffer) {
    free(ring_buffer);
}

#ifndef CUT_CHANNEL_H
#define CUT_CHANNEL_H

#include <pthread.h>
#include <ringbuffer.h>
typedef struct Channel{
    pthread_cond_t can_update_buffer;
    pthread_cond_t can_read_buffer;
    pthread_mutex_t mutex;
    RingBuffer* buffer;
} Channel;

char* channel_read(Channel* this_p){

}

#endif

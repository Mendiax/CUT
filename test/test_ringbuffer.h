#include <ktest/ktest.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <ringbuffer.h>

static void ringbufferTest_full(void);

static void ringbufferTest_full(void) {
    char test[2];
    RingBuffer* qtest = ring_buffer_create(sizeof(char) * 2, 4);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)0);
    ring_buffer_push(qtest,"0");
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)1);
    ring_buffer_push(qtest,"1");
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)2);
    ring_buffer_push(qtest,"2");
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)3);
    ring_buffer_push(qtest,"3");
    T_ASSERT_EQ(ring_buffer_push(qtest,"3"),-1);
    T_ASSERT_EQ(qtest->current_index, (size_t)0);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)4);


    ring_buffer_pop(qtest,test);
    T_ASSERT(!strcmp(test,"0"));
    ring_buffer_pop(qtest,test);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)2);
    ring_buffer_pop(qtest,test);
    ring_buffer_pop(qtest,test);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)0);
    T_ASSERT(ring_buffer_pop(qtest,test) == 0);

    ring_buffer_push(qtest,"0");
    ring_buffer_pop(qtest,test);
    ring_buffer_push(qtest,"2");
    ring_buffer_push(qtest,"3");
    T_ASSERT_EQ(qtest->current_index, (size_t)1);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)2);

    ring_buffer_destroy(qtest);
}

static void ringbufferTest_pointer(void);

static void ringbufferTest_pointer(void) {
    char test[2];
    RingBuffer* qtest = ring_buffer_create(sizeof(char) * 2, 4);
    memcpy(ring_buffer_push_pointer(qtest),"0",2);
    ring_buffer_push_pointer(qtest);
    ring_buffer_push_pointer(qtest);
    ring_buffer_push_pointer(qtest);
    T_ASSERT_EQ((void*)ring_buffer_push_pointer(qtest), (void*)0);
    T_ASSERT_EQ(qtest->current_index, (size_t)0);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)4);


    ring_buffer_pop(qtest,test);
    T_ASSERT(!strcmp(test,"0"));
    ring_buffer_pop(qtest,test);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)2);
    ring_buffer_pop(qtest,test);
    ring_buffer_pop(qtest,test);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)0);
    T_ASSERT(ring_buffer_pop(qtest,test) == 0);

    ring_buffer_destroy(qtest);
}

static void ringbufferTest_resize(void);

static void ringbufferTest_resize(void){
    char test[2];
    RingBuffer* qtest = ring_buffer_create(sizeof(char) * 2, 2);
    ring_buffer_push(qtest,"0");
    ring_buffer_push(qtest,"1");
    T_ASSERT_EQ(ring_buffer_push(qtest,"2"),-1);
    T_ASSERT_EQ(qtest->current_index, (size_t)0);
    T_ASSERT_EQ(qtest->current_queue_length, (size_t)2);

    T_ASSERT_EQ(ring_buffer_resize(&qtest,1),-1);
    T_ASSERT_EQ(ring_buffer_resize(&qtest,4),0);

    T_ASSERT_EQ(ring_buffer_push(qtest,"2"),0);
    T_ASSERT_EQ(ring_buffer_push(qtest,"3"),0);
    T_ASSERT_EQ(ring_buffer_push(qtest,"4"),-1);

    T_ASSERT_EQ(ring_buffer_pop(qtest,test),1);
    T_ASSERT(!strcmp(test,"0"));
    T_ASSERT_EQ(ring_buffer_pop(qtest,test),1);
    T_ASSERT(!strcmp(test,"1"));
    T_ASSERT_EQ(ring_buffer_pop(qtest,test),1);
    T_ASSERT(!strcmp(test,"2"));
    T_ASSERT_EQ(ring_buffer_pop(qtest,test),1);
    T_ASSERT(!strcmp(test,"3"));

    T_ASSERT_EQ(ring_buffer_pop(qtest,test),0);

    ring_buffer_destroy(qtest);
}

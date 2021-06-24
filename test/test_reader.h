#include <ktest/ktest.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <reader.h>
#include <ringbuffer.h>

static void reader_test1(void);

static void reader_test1(void) {
    ReaderThread* reader = reader_thread_create((void*) 0, (unsigned int) sysconf(_SC_NPROCESSORS_ONLN), 10);
    T_ASSERT(ring_buffer_is_empty(reader->reader_data->buffer));
    reader_thread_start(reader);

    sleep(1);

    int ret = reader_thread_stop(reader);
    T_ASSERT_EQ(ret, 0);
    T_ASSERT(ring_buffer_is_full(reader->reader_data->buffer));
    reader_thread_destroy(reader);
}

#ifndef CUT_READER_H
#define CUT_READER_H

#include <pthread.h>
#include <ringbuffer.h>
#include <signal.h>

#define RAW_DATA_LINE_SIZE (10 * 11 + 10 + 5 + 1) //10 * ull + 10 spaces + 5 for cpuXX + '0\'

/*
 * struct for holding gathered data and mutex
 */
typedef struct ReaderData
{
    pthread_cond_t can_update_buffer;
    pthread_cond_t can_read_buffer;
    pthread_mutex_t mutex;
    RingBuffer* buffer;
    size_t size;
    unsigned int number_of_threads;
    char pad[4];
} ReaderData;

/*
 * struct for holding thread data
 */
typedef struct ReaderThread
{
    pthread_t thread;
    ReaderData* reader_data;
    _Atomic(time_t) last_update;
    volatile sig_atomic_t should_end;
    char pad[4];
} ReaderThread;

/*
 * constructor
 */
ReaderData* reader_data_create(unsigned int thread_count, size_t buffer_length);

/*
 * destructor
 */
void reader_data_destroy(ReaderData* reader_data);

/*
 * updating data and writes to buffer
 * returns -1 if read data is NULL or couldn't open a file
 */
int reader_data_read_from_file(ReaderData* reader_data, char* buffer);

/*
 * reader thread function
 */
void* reader_thread_function(void* reader);

/*
 * constructor
 */
ReaderThread* reader_thread_create(unsigned int thread_count, size_t buffer_length);

/*
 * destructor
 */
void reader_thread_destroy(ReaderThread* reader);

/*
 * creating thread and starting it with reader data and reader thread function
 */
void reader_thread_start(ReaderThread* reader);

/*
 * waits for reader thread
 */
int reader_thread_join(ReaderThread* reader);

/*
 *  stops reader thread
 *  thread status 0 -> thread returned normally, -1 -> thread had an error
 */
int reader_thread_stop(ReaderThread* reader);


#endif //CUT_ANALYZER_H

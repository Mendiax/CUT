#ifndef CUT_LOGGER_H
#define CUT_LOGGER_H
#include <time.h>
#include <ringbuffer.h>
#include <pthread.h>
#include <signal.h>


typedef struct LoggerThread{
    pthread_cond_t can_update_buffer;
    pthread_cond_t can_read_buffer;
    pthread_mutex_t mutex_buffer;
    RingBuffer* buffer;
    size_t max_msg_length;
    pthread_t thread;
    _Atomic(time_t) last_update;
    volatile sig_atomic_t should_end;
    int return_status;
    char file_name[];
}LoggerThread;

void logger_thread_print(LoggerThread* logger, const char* format,...);

/*
 * thread function
 */
void* logger_thread_function(void* args);

/*
 * allocating all thread data
 */
LoggerThread* logger_thread_create(const char* file, size_t msg_max_size, size_t buffer_length);

/*
 * deallocating all thread data
 */
void logger_thread_destroy(LoggerThread* logger);

/*
 * starts thread
 */
void logger_thread_start(LoggerThread* logger);

/*
 *  joins thread
 */
int logger_thread_join(LoggerThread* logger);

/*
 * return thread status 0 -> thread returned normally, -1 -> thread had an error
 */
int logger_thread_stop(LoggerThread* logger);

#endif //CUT_LOGGER_H

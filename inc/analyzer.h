#ifndef CUT_ANALYZER_H
#define CUT_ANALYZER_H

#include <pthread.h>
#include <stdlib.h>
#include <reader.h>
#include <signal.h>

/*
 * data for calculating cpu usage
 */
typedef struct RawCpuData
{
    unsigned long long int idle, total;
}RawCpuData;

/*
 * data for analyzer
 */
typedef struct AnalyzerData
{
    pthread_cond_t can_update_buffer;
    pthread_cond_t can_read_buffer;
    pthread_mutex_t mutex;
    RawCpuData* raw_cpu_array2;
    RawCpuData* raw_cpu_array;
    RingBuffer* buffer;
    unsigned short thread_count;
    char pad[6];
} AnalyzerData;

/*
 * holds data for thread
 */
typedef struct AnalyzerThread
{
    pthread_t thread;
    ReaderData* reader_data;
    AnalyzerData* analyzer_data;
    volatile sig_atomic_t should_end;
    char pad[4];
} AnalyzerThread;

/*
 * return pointer to allocated structure
 */
AnalyzerData* analyzer_data_create(unsigned int thread_count, size_t buffer_length);

/*
 * deallocating structure
 */
void analyzer_data_destroy(AnalyzerData* analyzer_data);

/*
 * reads data from data pointer and writes it into returnBuffer
 */
int analyzer_data_unpack(RawCpuData* buffer_array, char* data, unsigned int thread_count);

/*
 * reads data from buffer and calculate cpu usage
 * returns cpu usage from 0 to 1
 */
float analyzer_get_cpu_usage(RawCpuData current_data, RawCpuData prev_data);

/*
 * analyzer thread function
 */
void* analyzer_thread_function(void* args);

/*
 * constructor
 */
AnalyzerThread* analyzer_thread_create(ReaderData* reader_data, unsigned short thread_count, size_t buffer_length);

/*
 * deallocating all thread data
 */
void analyzer_thread_destroy(AnalyzerThread* analyzer_thread);

/*
 * starts thread
 */
void analyzer_thread_start(AnalyzerThread* analyzer_thread);

/*
 * waits for analyzer thread
 */
int analyzer_thread_join(AnalyzerThread* analyzer_thread);

/*
 * stops analyzer thread
 * return thread status 0 -> thread returned normally, -1 -> thread had an error
 */
int analyzer_thread_stop(AnalyzerThread* analyzer_thread);

#endif //CUT_ANALYZER_H

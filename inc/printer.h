#ifndef CUT_PRINTER_H
#define CUT_PRINTER_H

#include <analyzer.h>
#include <signal.h>
#include <time.h>

/*
 * data for printer
 */
typedef struct PrinterThread
{
    pthread_t thread;
    AnalyzerData* analyzer_data;
    volatile sig_atomic_t should_end;
} PrinterThread;

/*
 * prints cpuUsageArray on terminal
 * n - size of array,
 * cpuUsage - array with cpu usage
 */
void printCpuUsage(size_t n, float cpu_usage[n]);

/*
 * thread function
 */
void* printer_thread_function(void*);
/*
 * allocating all thread data
 */
PrinterThread* printer_thread_create(AnalyzerData * analyzer_data);

/*
 * deallocating all thread data
 */
void printer_thread_destroy(PrinterThread* printer_thread);

/*
 * starts thread
 */
void printer_thread_start(PrinterThread* printer_thread);

/*
 * return thread status 0 -> thread returned normally, -1 -> thread had an error
 */
int printer_thread_stop(PrinterThread* printer_thread);

#endif //CUT_PRINTER_H

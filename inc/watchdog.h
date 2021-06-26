#ifndef CUT_WATCHDOG_H
#define CUT_WATCHDOG_H

#include <pthread.h>
#include <stdarg.h>
#include <signal.h>
#include <logger.h>

typedef struct WatchdogThread
{
    double timeout_time;
    pthread_t thread;
    LoggerThread* logger;
    volatile sig_atomic_t should_end;
    unsigned int number_of_threads;
    volatile _Atomic time_t* threads_last_update[];
} WatchdogThread;

WatchdogThread* watchdog_thread_create(LoggerThread* logger, double timeout, unsigned int  thread_count, ...);

void watchdog_thread_destroy(WatchdogThread* watchdog);

void* watchdog_thread_function(void* args);

void watchdog_thread_start(WatchdogThread* watchdog);

int watchdog_thread_join(WatchdogThread* watchdog);

int watchdog_thread_stop(WatchdogThread* watchdog);

#endif //CUT_WATCHDOG_H

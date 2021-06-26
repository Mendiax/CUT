#include <watchdog.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdatomic.h>

WatchdogThread* watchdog_thread_create(LoggerThread* logger,double timeout, unsigned int thread_count, ...) {
    WatchdogThread* new_watchdog = (WatchdogThread*) malloc(sizeof(WatchdogThread) + sizeof(volatile _Atomic time_t*) * thread_count);
    va_list args;
    va_start(args, thread_count);
    new_watchdog->logger = logger;
    new_watchdog->timeout_time = timeout;
    new_watchdog->should_end = 0;
    new_watchdog->number_of_threads = thread_count;
    for (size_t i = 0; i < thread_count; i++) {
        new_watchdog->threads_last_update[i] = va_arg(args,volatile _Atomic time_t*);
    }
    va_end(args);
    return new_watchdog;
}

void watchdog_thread_destroy(WatchdogThread* watchdog){
    free(watchdog);
}

void* watchdog_thread_function(void* args){
    WatchdogThread* thread = (WatchdogThread*)args;
    thread->return_status = 0;
    logger_thread_print(thread->logger,"[watchdog] started");
    while (!thread->should_end){
        usleep(10000);
        time_t currentTime = time(NULL);
        logger_thread_print(thread->logger,"[watchdog] checking threads");
        for(unsigned int i = 0; i < thread->number_of_threads; i++){
            if(difftime(currentTime,(atomic_load_explicit(thread->threads_last_update[i], memory_order_seq_cst))) > thread->timeout_time){
                thread->return_status = (int) i + 1;
                goto RET;
            }
        }
    }

RET:
    logger_thread_print(thread->logger,"[watchdog] exited %d", thread->return_status);
    pthread_exit(&thread->return_status);
}

void watchdog_thread_start(WatchdogThread* watchdog){
    pthread_create(&watchdog->thread, NULL, watchdog_thread_function, (void*) watchdog);
}

int watchdog_thread_join(WatchdogThread* watchdog){
    int* status = 0;
    pthread_join(watchdog->thread, (void**) &status);
    return (int) *status;
}

int watchdog_thread_stop(WatchdogThread* watchdog){
    watchdog->should_end = 1;
    return watchdog_thread_join(watchdog);
}

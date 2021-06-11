#include <watchdog.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

WatchdogThread* watchdog_thread_create(double timeout, size_t thread_count, ...) {
    WatchdogThread* newWatchDog = (WatchdogThread*) malloc(sizeof(WatchdogThread) + sizeof(volatile _Atomic time_t*) * thread_count);
    va_list args;
    va_start(args, thread_count);
    newWatchDog->timeout_time = timeout;
    newWatchDog->should_end = 0;
    newWatchDog->number_of_threads = thread_count;
    for (size_t i = 0; i < thread_count; i++) {
        newWatchDog->threads_last_update[i] = va_arg(args,
        volatile _Atomic time_t*);
    }
    va_end(args);
    return newWatchDog;
}

void watchdog_thread_destroy(WatchdogThread* watchdog){
    free(watchdog);
}

void* watchdog_thread_function(void* args){
    WatchdogThread* thread = (WatchdogThread*)args;
    sleep(1);
    long long returnStatus = 0;
    while (!thread->should_end){
        usleep(10000);
        time_t currentTime = time(NULL);
        for(size_t i = 0; i < thread->number_of_threads; i++){
            if(difftime(currentTime,*(thread->threads_last_update[i])) > thread->timeout_time){
                returnStatus = (int) i + 1;
                goto RET;
            }
        }
    }

RET:
    pthread_exit((void *) returnStatus);
}

void watchdog_thread_start(WatchdogThread* watchdog){
    pthread_create(&watchdog->thread, NULL, watchdog_thread_function, (void*) watchdog);
}

int watchdog_thread_join(WatchdogThread* watchdog){
    void* status = 0;
    pthread_join(watchdog->thread, (void**) &status);
    return (int) (long long) status;
}

int watchdog_thread_stop(WatchdogThread* watchdog){
    watchdog->should_end = 1;
    return watchdog_thread_join(watchdog);
}

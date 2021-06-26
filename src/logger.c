#include <logger.h>
#include <pthread.h>
#include <ringbuffer.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdatomic.h>

void logger_thread_print(LoggerThread* logger, const char* format, ...) {
    if(!logger){
        return;
    }
    char msg[logger->max_msg_length];
    va_list argptr;
    va_start(argptr, format);
    snprintf(msg,logger->max_msg_length,format,argptr);
    va_end(argptr);

    pthread_mutex_lock(&logger->mutex_buffer);
    if (ring_buffer_is_full(logger->buffer)) {
        pthread_cond_wait(&logger->can_update_buffer, &logger->mutex_buffer);
    }
    ring_buffer_push_resize(logger->buffer, msg,1.5f);
    pthread_cond_signal(&logger->can_read_buffer);
    pthread_mutex_unlock(&logger->mutex_buffer);
}

void* logger_thread_function(void* args) {
    LoggerThread* thread = (LoggerThread*) args;
    thread->return_status = 0;
    char msg[thread->buffer->size_of_element];
    struct tm* local;
    time_t timer;
    size_t last_length = 0;

    FILE* file = fopen(thread->file_name,"w");
    if(!file){
        perror("Cannot open a file!\n");
        thread->return_status = 1;
        pthread_exit(&thread->return_status);
    }
    while (!thread->should_end || last_length) {
        pthread_mutex_lock(&thread->mutex_buffer);
        if (ring_buffer_is_empty(thread->buffer)) {
            pthread_cond_wait(&thread->can_read_buffer, &thread->mutex_buffer);
        }

        if (!ring_buffer_pop(thread->buffer, msg)) {
            pthread_mutex_unlock(&thread->mutex_buffer);
            if (!thread->should_end) {
                thread->return_status = 1;
            }
            break;
        }
        last_length = thread->buffer->current_queue_length;

        pthread_cond_signal(&thread->can_update_buffer);
        pthread_mutex_unlock(&thread->mutex_buffer);

        timer = time(NULL);
        atomic_store_explicit(&thread->last_update, timer, memory_order_seq_cst);
        local = localtime(&timer);
        msg[thread->buffer->size_of_element - 1] = 0;
        fprintf(file,"%02d:%02d:%02d %s\n",local->tm_hour,local->tm_min,local->tm_sec, msg);
    }
    fclose(file);
    pthread_exit( &thread->return_status);
}

LoggerThread* logger_thread_create(const char* file, size_t msg_max_size, size_t buffer_length){
    LoggerThread* logger = malloc(sizeof (LoggerThread) + strlen(file) + 1);
    logger->buffer = ring_buffer_create(sizeof(char) * msg_max_size, buffer_length);
    pthread_mutex_init(&logger->mutex_buffer, NULL);
    pthread_cond_init(&logger->can_update_buffer, NULL);
    pthread_cond_init(&logger->can_read_buffer, NULL);
    logger->max_msg_length = msg_max_size;
    atomic_store_explicit(&logger->last_update, 0, memory_order_seq_cst);
    logger->should_end = 0;
    strcpy(logger->file_name,file);

    return logger;
}

void logger_thread_destroy(LoggerThread* logger){
    pthread_mutex_lock(&logger->mutex_buffer);
    ring_buffer_destroy(logger->buffer);
    pthread_mutex_unlock(&logger->mutex_buffer);

    pthread_mutex_destroy(&logger->mutex_buffer);
    pthread_cond_destroy(&logger->can_update_buffer);
    pthread_cond_destroy(&logger->can_read_buffer);

    free(logger);
}

void logger_thread_start(LoggerThread* logger){
    logger->should_end = 0;
    pthread_create(&logger->thread, NULL, logger_thread_function, (void*) logger);
}

int logger_thread_join(LoggerThread* logger){
    int* status = 0;
    pthread_join(logger->thread, (void **) &status);
    return *status;
}

int logger_thread_stop(LoggerThread* logger){
    logger->should_end = 1;
    pthread_cond_signal(&logger->can_read_buffer);
    return logger_thread_join(logger);
}

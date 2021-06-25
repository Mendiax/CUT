#include <analyzer.h>
#include <printer.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdatomic.h>

void print_cpu_usage(size_t n, float * cpu_usage){
    printf("\x1b[2A");
    printf("\x1b[0J");
    printf("CPU  : ");
    for(size_t i = 0; i < n; i++) {
        printf("%5zu ", i);
    }
    printf("\n\x1b[0J");
    printf("Usage: ");
    for(size_t i = 0; i < n; i++) {
        printf("%5.1f ", cpu_usage[i] * 100.0f);
    }
    printf("\n");
}

void* printer_thread_function(void* args) {
    PrinterThread* thread = (PrinterThread*) args;
    float readerBuffer[thread->analyzer_data->buffer->size_of_element];
    atomic_store_explicit(&thread->last_update, time(NULL), memory_order_seq_cst);
    printf("CPU avg usage (1s)\n\n\n");
    while (1) {
        pthread_mutex_lock(&thread->analyzer_data->mutex);
        if (ring_buffer_is_empty(thread->analyzer_data->buffer)) {
            pthread_cond_wait(&thread->analyzer_data->can_read_buffer, &thread->analyzer_data->mutex);
        }
        if (thread->should_end) {
            pthread_mutex_unlock(&thread->analyzer_data->mutex);
            pthread_exit(0);
        }

        if (!ring_buffer_pop(thread->analyzer_data->buffer, (char *)readerBuffer)) {
            pthread_exit((void*) -1);
        }

        pthread_cond_signal(&thread->analyzer_data->can_update_buffer);
        pthread_mutex_unlock(&thread->analyzer_data->mutex);
        logger_thread_print(thread->logger,"[printer] printing\n");
        print_cpu_usage(thread->analyzer_data->thread_count, readerBuffer);
        atomic_store_explicit(&thread->last_update, time(NULL), memory_order_seq_cst);
    }
}

PrinterThread* printer_thread_create(LoggerThread* logger,AnalyzerData* analyzer_data) {
    PrinterThread* printer = malloc(sizeof(PrinterThread));
    printer->logger = logger;
    printer->should_end = 0;
    printer->analyzer_data = analyzer_data;
    return printer;
}

void printer_thread_destroy(PrinterThread* printer_thread) {
    free(printer_thread);
}

void printer_thread_start(PrinterThread* printer_thread) {
    pthread_create(&printer_thread->thread, NULL, printer_thread_function, (void*) printer_thread);
}

int printer_thread_stop(PrinterThread* printer_thread) {
    printer_thread->should_end = 1;
    pthread_cond_signal(&printer_thread->analyzer_data->can_read_buffer);
    void* status = 0;
    pthread_join(printer_thread->thread, (void**) &status);

    return (int) (long long) status;
}

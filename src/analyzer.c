#include <analyzer.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

int analyzer_data_unpack(RawCpuData* buffer_array, char* data, unsigned int thread_count) {
    if (data == NULL) {
        return 0;
    }
    unsigned long long int non_idle;
    //variables for reading
    unsigned long long int user_time, nice_time, system_time, idle_time,
            io_wait, irq, irq_soft, steal, guest, guest_nice;
    for (unsigned int i = 0; i < thread_count; i++) {
        if (!strlen(data)) {
            return 0;
        }
        unsigned int cpuid;
        int scanned = sscanf(data, "cpu%4d %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
                             &cpuid, &user_time,
                             &nice_time, &system_time, &idle_time,
                             &io_wait, &irq,
                             &irq_soft, &steal, &guest,
                             &guest_nice);
        assert(cpuid == i);
        assert(scanned == 11);
        data = strchr(data, '\n') + 1;

        non_idle = user_time + nice_time + system_time + irq + irq_soft + steal;
        buffer_array[i].idle = idle_time + io_wait;
        buffer_array[i].total = buffer_array[i].idle + non_idle;
    }
    return 1;
}

float analyzer_get_cpu_usage(RawCpuData current_data, RawCpuData prev_data) {
    unsigned long long int total = current_data.total - prev_data.total;
    unsigned long long int idle = current_data.idle - prev_data.idle;

    float cpu_percentage = (float) (total - idle) / (float) total;
    return cpu_percentage;
}

AnalyzerData* analyzer_data_create(unsigned int thread_count, size_t buffer_length) {
    AnalyzerData* analyzer_data = (AnalyzerData*) malloc(sizeof(AnalyzerData));
    analyzer_data->thread_count = thread_count;
    analyzer_data->raw_cpu_array = (RawCpuData*) calloc(thread_count, sizeof(RawCpuData));
    analyzer_data->raw_cpu_array2 = (RawCpuData*) calloc(thread_count, sizeof(RawCpuData));

    pthread_mutex_init(&analyzer_data->mutex, NULL);
    pthread_cond_init(&analyzer_data->can_update_buffer, NULL);
    pthread_cond_init(&analyzer_data->can_read_buffer, NULL);

    analyzer_data->buffer = ring_buffer_create(sizeof(float) * thread_count, buffer_length);
    return analyzer_data;
}

void analyzer_data_destroy(AnalyzerData* analyzer_data) {
    pthread_mutex_lock(&analyzer_data->mutex);
    free(analyzer_data->raw_cpu_array);
    free(analyzer_data->raw_cpu_array2);
    ring_buffer_destroy(analyzer_data->buffer);
    pthread_mutex_unlock(&analyzer_data->mutex);

    pthread_mutex_destroy(&analyzer_data->mutex);
    pthread_cond_destroy(&analyzer_data->can_update_buffer);
    pthread_cond_destroy(&analyzer_data->can_read_buffer);
    free(analyzer_data);
}

void* analyzer_thread_function(void* args) {
    AnalyzerThread* thread = (AnalyzerThread*) args;
    char reader_buffer[thread->reader_data->buffer->size_of_element];

    while (1) {
        pthread_mutex_lock(&thread->reader_data->mutex);
        if (ring_buffer_is_empty(thread->reader_data->buffer)) {
            pthread_cond_wait(&thread->reader_data->can_read_buffer, &thread->reader_data->mutex);
        }
        if (thread->should_end) {
            pthread_mutex_unlock(&thread->reader_data->mutex);
            pthread_exit(0);
        }
        if (!ring_buffer_pop(thread->reader_data->buffer, reader_buffer)) {
            pthread_exit((void*) -1);
        }

        pthread_cond_signal(&thread->reader_data->can_update_buffer);
        pthread_mutex_unlock(&thread->reader_data->mutex);

        //update analyzer buffer
        pthread_mutex_lock(&thread->analyzer_data->mutex);
        if (ring_buffer_is_full(thread->analyzer_data->buffer)) {
            pthread_cond_wait(&thread->analyzer_data->can_update_buffer, &thread->analyzer_data->mutex);
        }
        if (thread->should_end) {
            pthread_mutex_unlock(&thread->analyzer_data->mutex);
            pthread_exit(0);
        }

        //calculate new data and put in buffer
        float* cpu_usage_buffer = (float*) ring_buffer_push_pointer(thread->analyzer_data->buffer);
        if (!analyzer_data_unpack(thread->analyzer_data->raw_cpu_array, reader_buffer, thread->analyzer_data->thread_count)) {
            pthread_exit((void*) -1);
        }
        for (unsigned int i = 0; i < thread->analyzer_data->thread_count; i++) {
            cpu_usage_buffer[i] = analyzer_get_cpu_usage(thread->analyzer_data->raw_cpu_array[i], thread->analyzer_data->raw_cpu_array2[i]);
        }
        RawCpuData* tmp = thread->analyzer_data->raw_cpu_array;
        thread->analyzer_data->raw_cpu_array = thread->analyzer_data->raw_cpu_array2;
        thread->analyzer_data->raw_cpu_array2 = tmp;

        pthread_cond_signal(&thread->analyzer_data->can_read_buffer);
        pthread_mutex_unlock(&thread->analyzer_data->mutex);
    }
}

AnalyzerThread* analyzer_thread_create(ReaderData* reader_data, unsigned short thread_count, size_t buffer_length) {
    AnalyzerThread* analyzer_thread = malloc(sizeof(AnalyzerThread));
    analyzer_thread->should_end = 0;
    analyzer_thread->analyzer_data = analyzer_data_create(thread_count, buffer_length);
    analyzer_thread->reader_data = reader_data;
    return analyzer_thread;
}

void analyzer_thread_destroy(AnalyzerThread* analyzer_thread) {
    analyzer_data_destroy(analyzer_thread->analyzer_data);
    free(analyzer_thread);
}

void analyzer_thread_start(AnalyzerThread* analyzer_thread) {
    pthread_create(&analyzer_thread->thread, NULL, analyzer_thread_function, (void*) analyzer_thread);
}

int analyzer_thread_join(AnalyzerThread* analyzer_thread) {
    void* status = 0;
    pthread_join(analyzer_thread->thread, (void**) &status);

    return (int) (long long) status;
}

int analyzer_thread_stop(AnalyzerThread* analyzer_thread) {
    analyzer_thread->should_end = 1;
    pthread_cond_signal(&analyzer_thread->analyzer_data->can_update_buffer);
    pthread_cond_signal(&analyzer_thread->reader_data->can_read_buffer);
    void* status = 0;
    pthread_join(analyzer_thread->thread, (void**) &status);

    return analyzer_thread_join(analyzer_thread);
}
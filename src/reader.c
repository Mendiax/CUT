#include <reader.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int reader_data_read_from_file(ReaderData* reader_data, char* buffer) {
    if (buffer == NULL) {
        return 0;
    }
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        return 0;
    }
    char* ptr = buffer;
    if (fgets(ptr, RAW_DATA_LINE_SIZE, file) == NULL) {
        goto ret;
    }
    for (unsigned int i = 0; i < reader_data->number_of_threads; i++) {
        if (fgets(ptr, RAW_DATA_LINE_SIZE, file) == NULL) {
            goto ret;
        }
        ptr += strlen(ptr) * sizeof(char);
    }
    ret:
    fclose(file);
    return 1;
}

ReaderData* reader_data_create(unsigned int thread_count, size_t buffer_length) {
    ReaderData* reader_data = (ReaderData*) malloc(sizeof(ReaderData));
    reader_data->number_of_threads = thread_count;
    reader_data->size = RAW_DATA_LINE_SIZE * reader_data->number_of_threads;
    reader_data->buffer = ring_buffer_create(sizeof(char) * reader_data->size, buffer_length);

    pthread_mutex_init(&reader_data->mutex, NULL);
    pthread_cond_init(&reader_data->can_update_buffer, NULL);
    pthread_cond_init(&reader_data->can_read_buffer, NULL);

    return reader_data;
}

void reader_data_destroy(ReaderData* reader_data) {
    pthread_mutex_lock(&reader_data->mutex);
    ring_buffer_destroy(reader_data->buffer);
    pthread_mutex_unlock(&reader_data->mutex);

    pthread_mutex_destroy(&reader_data->mutex);
    pthread_cond_destroy(&reader_data->can_update_buffer);
    pthread_cond_destroy(&reader_data->can_read_buffer);

    free(reader_data);

}

void* reader_thread_function(void* args) {
    ReaderThread* thread = (ReaderThread*) args;
    int return_status = 0;
    while (!thread->should_end) {
        pthread_mutex_lock(&thread->reader_data->mutex);
        if (ring_buffer_is_full(thread->reader_data->buffer)) {
            pthread_cond_wait(&thread->reader_data->can_update_buffer, &thread->reader_data->mutex);
        }
        void* next_free_space = ring_buffer_push_pointer(thread->reader_data->buffer);
        if (!reader_data_read_from_file(thread->reader_data, next_free_space)) {
            pthread_mutex_unlock(&thread->reader_data->mutex);
            if (!thread->should_end) {
                return_status = 1;
            }
            break;
        }

        pthread_cond_signal(&thread->reader_data->can_read_buffer);
        pthread_mutex_unlock(&thread->reader_data->mutex);
        usleep(10000);
    }
    pthread_exit((void*) return_status);

}

ReaderThread* reader_thread_create(unsigned int thread_count, size_t buffer_length) {
    ReaderThread* reader_data = malloc(sizeof(ReaderThread));
    reader_data->should_end = 0;
    reader_data->reader_data = reader_data_create(thread_count, buffer_length);
    return reader_data;
}

void reader_thread_destroy(ReaderThread* reader) {
    reader_data_destroy(reader->reader_data);
    free(reader);
}

void reader_thread_start(ReaderThread* reader) {
    reader->should_end = 0;
    pthread_create(&reader->thread, NULL, reader_thread_function, (void*) reader);
}

int reader_thread_join(ReaderThread* reader) {
    int* status = 0;
    pthread_join(reader->thread, (void**) &status);
    return (int) (long long)status;
}

int reader_thread_stop(ReaderThread* reader) {
    reader->should_end = 1;
    pthread_cond_signal(&reader->reader_data->can_update_buffer);

    return reader_thread_join(reader);
}

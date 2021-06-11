#include <ktest/ktest.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <reader.h>
#include <analyzer.h>
#include <printer.h>
#include <watchdog.h>

static void watchdog_test_reader();

static void watchdog_test_analyzer();

static void watchdog_test_reader() {
    unsigned int t = sysconf(_SC_NPROCESSORS_ONLN);
    ReaderThread* reader = reader_thread_create(t, 10);
    AnalyzerThread* analyzer = analyzer_thread_create(reader->reader_data, t, 10);
    PrinterThread* printer = printer_thread_create(analyzer->analyzer_data);
    WatchdogThread* watchdog = watchdog_thread_create(2.0, 3, &reader->last_update, &analyzer->last_update, &printer->last_update);

    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);
    watchdog_thread_start(watchdog);

    sleep(1);
    pthread_mutex_lock(&reader->reader_data->mutex);

    int retWatchdog = watchdog_thread_join(watchdog);
    pthread_mutex_unlock(&reader->reader_data->mutex);
    T_ASSERT(retWatchdog == 3 || retWatchdog == 2 || retWatchdog == 1);

    int ret_printer = printer_thread_stop(printer);
    T_ASSERT_EQ(ret_printer, 0);
    printer_thread_destroy(printer);

    int ret_analyzer = analyzer_thread_stop(analyzer);
    T_ASSERT_EQ(ret_analyzer, 0);
    analyzer_thread_destroy(analyzer);

    int ret_reader = reader_thread_stop(reader);
    T_ASSERT_EQ(ret_reader, 0);
    reader_thread_destroy(reader);

    watchdog_thread_destroy(watchdog);
}


static void watchdog_test_analyzer() {
    unsigned int t = sysconf(_SC_NPROCESSORS_ONLN);
    ReaderThread* reader = reader_thread_create(t, 10);
    AnalyzerThread* analyzer = analyzer_thread_create(reader->reader_data, t, 10);
    PrinterThread* printer = printer_thread_create(analyzer->analyzer_data);
    WatchdogThread* watchdog = watchdog_thread_create(2.0, 3, &reader->last_update, &analyzer->last_update, &printer->last_update);

    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);
    watchdog_thread_start(watchdog);

    sleep(1);
    pthread_mutex_lock(&printer->analyzer_data->mutex);

    int retWatchdog = watchdog_thread_join(watchdog);
    pthread_mutex_unlock(&printer->analyzer_data->mutex);
    T_ASSERT(retWatchdog == 3 || retWatchdog == 2 || retWatchdog == 1);

    int ret_printer = printer_thread_stop(printer);
    T_ASSERT_EQ(ret_printer, 0);
    printer_thread_destroy(printer);

    int ret_analyzer = analyzer_thread_stop(analyzer);
    T_ASSERT_EQ(ret_analyzer, 0);
    analyzer_thread_destroy(analyzer);

    int ret_reader = reader_thread_stop(reader);
    T_ASSERT_EQ(ret_reader, 0);
    reader_thread_destroy(reader);

    watchdog_thread_destroy(watchdog);
}

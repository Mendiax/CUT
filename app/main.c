#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <reader.h>
#include <analyzer.h>
#include <printer.h>
#include <watchdog.h>

static ReaderThread* reader;
static AnalyzerThread* analyzer;
static PrinterThread* printer;
static WatchdogThread* watchdog;
static LoggerThread* logger;

static const char* const threadsNames[] = {"watchdog", "reader", "analyzer", "printer"};

static void stop_all() {
    printer_thread_stop(printer);
    analyzer_thread_stop(analyzer);
    reader_thread_stop(reader);
    watchdog_thread_stop(watchdog);
    logger_thread_stop(logger);
}

static void destroy_all() {
    printer_thread_destroy(printer);
    analyzer_thread_destroy(analyzer);
    reader_thread_destroy(reader);
    watchdog_thread_destroy(watchdog);
    logger_thread_destroy(logger);
}

__attribute__((noreturn)) static void int_handler() {
    stop_all();
    destroy_all();
    printf("\nStopping program\n");
    sleep(1);
    exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGINT, int_handler);
    logger = logger_thread_create("log.txt", 100, 10);
    unsigned short t = (unsigned short) sysconf(_SC_NPROCESSORS_ONLN);
    reader = reader_thread_create(logger, (unsigned short) t, 10);
    analyzer = analyzer_thread_create(logger, reader->reader_data, (unsigned short) t, 10);
    printer = printer_thread_create(logger, analyzer->analyzer_data);
    watchdog = watchdog_thread_create(logger, 2.0, 3, &reader->last_update, &analyzer->last_update, &printer->last_update);

    logger_thread_start(logger);
    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);
    watchdog_thread_start(watchdog);

    int ret_watchdog = watchdog_thread_join(watchdog);
    printf("%s thread is not responding\nexiting program\n", threadsNames[ret_watchdog]);
    logger_thread_print(logger, "%s thread is not responding\nexiting program\n", threadsNames[ret_watchdog]);

    stop_all();
    destroy_all();

    return EXIT_FAILURE;
}

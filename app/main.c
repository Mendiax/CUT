#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <reader.h>
#include <analyzer.h>
#include <printer.h>
#include <watchdog.h>

static ReaderThread* reader;
static AnalyzerThread* analyzer;
static PrinterThread* printer;
static WatchdogThread* watchdog;

const char*  const threadsNames[] = {"reader", "analyzer", "printer", "watchdog"};

volatile sig_atomic_t end = 0;

static void intHandler(int dummy) {
    int ret_printer = printer_thread_stop(printer);
    int ret_analyzer = analyzer_thread_stop(analyzer);
    int ret_reader = reader_thread_stop(reader);
    int ret_watchdog = watchdog_thread_stop(watchdog);

    printer_thread_destroy(printer);
    analyzer_thread_destroy(analyzer);
    reader_thread_destroy(reader);
    watchdog_thread_destroy(watchdog);

    exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGINT, intHandler);

    unsigned short t = (unsigned short)sysconf(_SC_NPROCESSORS_ONLN);
    reader = reader_thread_create((unsigned short) t, 10);
    analyzer = analyzer_thread_create(reader->reader_data, (unsigned short) t, 10);
    printer = printer_thread_create(analyzer->analyzer_data);
    watchdog = watchdog_thread_create(2.0, 3, &reader->last_update, &analyzer->last_update, &printer->last_update);


    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);
    watchdog_thread_start(watchdog);

    int ret_watchdog = watchdog_thread_join(watchdog);
    printf("%s thread is not responding\nexiting program\n",threadsNames[ret_watchdog]);
    int ret_printer = printer_thread_stop(printer);
    int ret_analyzer = analyzer_thread_stop(analyzer);
    int ret_reader = reader_thread_stop(reader);

    printer_thread_destroy(printer);
    analyzer_thread_destroy(analyzer);
    reader_thread_destroy(reader);
    watchdog_thread_destroy(watchdog);
    return 0;
}


#include <stdio.h>
#include <unistd.h>
#include <reader.h>
#include <printer.h>
#include <signal.h>
#include <assert.h>

static ReaderThread* reader;
static AnalyzerThread* analyzer;
static PrinterThread* printer;

volatile sig_atomic_t end = 0;

static void intHandler(int dummy) {
    int ret_printer = printer_thread_stop(printer);
    int ret_analyzer = analyzer_thread_stop(analyzer);
    int ret_reader = reader_thread_stop(reader);

    printer_thread_destroy(printer);
    analyzer_thread_destroy(analyzer);
    reader_thread_destroy(reader);

    end = 1;
}

int main(void) {

    signal(SIGINT, intHandler);

    unsigned short t = (unsigned short)sysconf(_SC_NPROCESSORS_ONLN);
    reader = reader_thread_create((unsigned short) t, 10);
    analyzer = analyzer_thread_create(reader->reader_data, (unsigned short) t, 10);
    printer = printer_thread_create(analyzer->analyzer_data);


    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);


    while (!end){
        sleep(1);
    }
    return 0;
}


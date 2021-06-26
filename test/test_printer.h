#include <ktest/ktest.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <reader.h>
#include <analyzer.h>
#include <printer.h>
static void printer_test1(void);

static void printer_test1(void) {
    unsigned int t = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
    ReaderThread* reader = reader_thread_create((void*)0,t, 10);
    AnalyzerThread* analyzer = analyzer_thread_create((void*)0,reader->reader_data,t, 10);
    PrinterThread* printer = printer_thread_create((void*)0,analyzer->analyzer_data);

    reader_thread_start(reader);
    analyzer_thread_start(analyzer);
    printer_thread_start(printer);

    sleep(5);

    int ret_printer = printer_thread_stop(printer);
    T_ASSERT_EQ(ret_printer, 0);
    printer_thread_destroy(printer);

    int ret_analyzer = analyzer_thread_stop(analyzer);
    T_ASSERT_EQ(ret_analyzer, 0);
    analyzer_thread_destroy(analyzer);

    int ret_reader = reader_thread_stop(reader);
    T_ASSERT_EQ(ret_reader, 0);
    reader_thread_destroy(reader);
}

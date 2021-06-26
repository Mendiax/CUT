#include <ktest/ktest.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <reader.h>
#include <analyzer.h>

static void analyzer_test1(void);

static void analyzer_test1(void) {
    ReaderThread* reader = reader_thread_create((void*)0,(unsigned int) sysconf(_SC_NPROCESSORS_ONLN), 10);
    AnalyzerThread* analyzer = analyzer_thread_create((void*)0,reader->reader_data,(unsigned short) sysconf(_SC_NPROCESSORS_ONLN), 10);
    reader_thread_start(reader);
    analyzer_thread_start(analyzer);

    sleep(1);

    int ret_analyzer = analyzer_thread_stop(analyzer);
    T_ASSERT_EQ(ret_analyzer, 0);
    analyzer_thread_destroy(analyzer);

    int ret_reader = reader_thread_stop(reader);
    T_ASSERT_EQ(ret_reader, 0);
    reader_thread_destroy(reader);
}

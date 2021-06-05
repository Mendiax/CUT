#include <ktest/ktest.h>
#include "test_ringbuffer.h"
#include "test_reader.h"
#include "test_analyzer.h"

int main(void) {
    TEST_SUITE_INIT("RINGBUFFER TEST");
    TEST_CASE_RUN(ringbufferTest_full());
    TEST_CASE_RUN(ringbufferTest_pointer());
    TEST_SUITE_SUMMARY();

    int result = TEST_SUITE_GET_RESULT();

    TEST_SUITE_INIT("READER TEST");
    TEST_CASE_RUN(reader_test1());
    TEST_SUITE_SUMMARY();
    result += TEST_SUITE_GET_RESULT();

    TEST_SUITE_INIT("ANALYZER TEST");
    TEST_CASE_RUN(analyzer_test1());
    TEST_SUITE_SUMMARY();
    result += TEST_SUITE_GET_RESULT();

    return result;
}

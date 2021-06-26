#include <logger.h>
#include <time.h>
#include <stdio.h>

#define size (20+9)
static void logger_test1(){
    LoggerThread* logger;
    logger = logger_thread_create("test.txt",20,5);

    logger_thread_start(logger);
    for(size_t i = 0; i < 10; i++){
        logger_thread_print(logger,"test");
    }
    sleep(1);
    logger_thread_stop(logger);
    FILE* file = fopen(logger->file_name,"r");
    T_ASSERT_PTR_NOT_NULL(file);
    char buffer[size];
    int h,m,s;
    char msg[20];
    for(size_t i = 0; i < 10; i++){
        T_ASSERT_PTR_NOT_NULL(fgets(buffer, size, file));
        sscanf(buffer,"%02d:%02d:%02d %s\n",&h,&m,&s,msg);
        T_ASSERT(strcmp(msg,"test") == 0);
    }

    fclose(file);
    remove(logger->file_name);
    logger_thread_destroy(logger);
}
#undef size

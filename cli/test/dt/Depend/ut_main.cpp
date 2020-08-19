#include "gtest/gtest.h"
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>

using namespace testing;

void handler(int sig)
{
    void *array[20];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 20);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(sig);
}

int main(int argc, char* argv[])
{
    signal(SIGABRT, handler);
    signal(SIGFPE, handler);
    signal(SIGILL, handler);
    signal(SIGINT, handler);
    signal(SIGSEGV, handler);
    signal(SIGTERM, handler);

    //testing::InitGoogleTest(&argc, argv);
    int result = testing::Init_UT(argc, argv,true);
    
    return result;
}

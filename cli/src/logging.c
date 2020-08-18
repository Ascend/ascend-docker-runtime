/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具日志模块
*/
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "securec.h"
#include "basic.h"
#include "options.h"

static int g_pid = -1;
static FILE *g_logFile = NULL;

void SetPidForLog(int pid)
{
    g_pid = pid;
}

int OpenLog(const char *logFile)
{
    char realPath[PATH_MAX] = {0};

    if (!IsOptionVerboseSet()) { // 日志开关
        return 0;
    }

    if (realpath(logFile, realPath) == NULL && errno != ENOENT) {
        LogError("error: cannot canonicalize log file path %s.", logFile);
        return -1;
    }

    g_logFile = fopen((const char *)realPath, "ae");
    if (g_logFile == NULL) {
        LogError("error: failed to open log file %s.", realPath);
        return -1;
    }

    return 0;
}

void CloseLog()
{
    if (IsOptionVerboseSet() && g_logFile != NULL) {
        (void)fclose(g_logFile);
        g_logFile = NULL;
    }
}

static void WriteLog(char level, const char *fmt, va_list args)
{
    struct timeval tv = {0};
    struct tm *tm = NULL;
    char buf[BUF_SIZE] = {0};

    if (g_logFile == NULL) {
        return;
    }

    if (gettimeofday(&tv, NULL) < 0 ||
        (tm = gmtime(&tv.tv_sec)) == NULL ||
        strftime(buf, sizeof(buf), "%m%d %T", tm) == 0) {
        strcpy_s(buf, sizeof(buf), "0000 00:00:00");
    }

    fprintf(g_logFile, "[%c %s.%06ld %d] ", level, buf, tv.tv_usec, g_pid);
    vfprintf(g_logFile, fmt, args);
    fputc('\n', g_logFile);
}

void LogError(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    va_list argsCopy;
    va_copy(argsCopy, args);
    WriteLog('E', fmt, argsCopy);
    va_end(argsCopy);

    vfprintf(stderr, fmt, args);
    va_end(args);
}

void LogInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    va_list argsCopy;
    va_copy(argsCopy, args);
    WriteLog('I', fmt, argsCopy);
    va_end(argsCopy);

    vfprintf(stdout, fmt, args);
    va_end(args);
}

void LogWarning(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    va_list argsCopy;
    va_copy(argsCopy, args);
    WriteLog('W', fmt, argsCopy);
    va_end(argsCopy);

    vfprintf(stderr, fmt, args);
    va_end(args);
}
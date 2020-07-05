/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具日志模块
*/
#include "logging.h"

#include <stdio.h>
#include <stdarg.h>

void logError(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stderr, fmt, args);
    va_end(args);
}

void logInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    fprintf(stdout, fmt, args);
    va_end(args);
}
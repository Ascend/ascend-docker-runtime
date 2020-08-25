/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具日志模块头文件
*/
#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdio.h>
#include <stdarg.h>
#include "securec.h"
#include "basic.h"

void SetPidForLog(int pid);
int OpenLog(const char *logFile);
void CloseLog();
void WriteLog(char level, const char *content);

#define LOG(level, fmt, ...)                                          \
    do {                                                              \
        char _content[BUF_SIZE] = {0};                                \
        int _ret = sprintf_s(_content, BUF_SIZE, fmt, ##__VA_ARGS__); \
        if (_ret < 0) {                                               \
            fprintf(stderr, "cannot assemble log content");           \
        } else {                                                      \
            WriteLog(level, (const char *)_content);                  \
            fprintf(stderr, "%s", (const char *)_content);            \
        }                                                             \
    } while (0)

#define LOG_ERROR(fmt, ...) LOG('E', fmt, ##__VA_ARGS__)

#define LOG_WARNING(fmt, ...) LOG('W', fmt, ##__VA_ARGS__)

#endif

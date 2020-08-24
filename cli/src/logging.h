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

#define LOG(level, fmt, ...)                                        \
    do {                                                            \
        char content[BUF_SIZE] = {0};                               \
        int ret = sprintf_s(content, BUF_SIZE, fmt, ##__VA_ARGS__); \
        if (ret < 0) {                                              \
            break;                                                  \
        }                                                           \
        WriteLog(level, (const char *)content);                     \
        fprintf(stderr, "%s", (const char *)content);               \
    } while (0)

#define LOG_ERROR(fmt, ...)           \
    do {                              \
        LOG('E', fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_WARNING(fmt, ...)         \
    do {                              \
        LOG('W', fmt, ##__VA_ARGS__); \
    } while (0)

#endif

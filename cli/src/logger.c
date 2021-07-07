/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli日志模块
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "securec.h"
#include "basic.h"

#define FILE_MAX_SIZE (1024 * 1024 * 10)
#define LOG_PATH_DIR "/var/log/"
#define TEMP_BUFFER 30
#define YEAR_OFFSET 1900
#define MONTH_OFFSET 1
#define LEVEL_LENGTH 20

int GetCurrentLocalTime(char* buffer, int length)
{
    time_t rawtime;
    struct tm* timeinfo = NULL;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return sprintf_s(buffer,
                     TEMP_BUFFER,
                     "[%04d-%02d-%02d %02d:%02d:%02d]",
                     (timeinfo->tm_year + YEAR_OFFSET),
                     (timeinfo->tm_mon + MONTH_OFFSET),
                     (timeinfo->tm_mday),
                     (timeinfo->tm_hour),
                     (timeinfo->tm_min),
                     (timeinfo->tm_sec));
}

long GetLogSize(char* filename)
{
    long length = 0;
    FILE *fp = NULL;
    fp = fopen(filename, "rb");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
    }
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
    return length;
}

void LogLoop(char* filename)
{
    char* loopPath = LOG_PATH_DIR"docker-runtime-log.log.1";
    int exist;
    exist = access(loopPath, 0);
    if (exist == 0) {
        unlink(loopPath);
    }
    rename(filename, loopPath);
}

void WriteLogFile(char* filename, long maxSize, char* buffer, unsigned bufferSize)
{
    if (filename != NULL && buffer != NULL) {
        long length = GetLogSize(filename);
        if (length > maxSize) {
            LogLoop(filename);
        }
        FILE *fp;
        fp = fopen(filename, "a+");
        if (fp != NULL) {
            int ret;
            char now[TEMP_BUFFER] = {0};
            ret = GetCurrentLocalTime(now, sizeof(now) / sizeof(char));
            if (ret < 0) {
                fclose(fp);
                return;
            }
            fwrite(now, strlen(now), 1, fp);
            fwrite(buffer, bufferSize, 1, fp);
            fclose(fp);
            fp = NULL;
        }
    }
}

void Logger(const char *msg, int level, int length)
{
    if (msg == NULL) {
        return;
    }
    int iret;
    char *logPath = LOG_PATH_DIR"docker-runtime-log.log";
    int destMax = length + LEVEL_LENGTH;
    if (destMax <= 0) {
        return;
    }
    char* buffer = malloc(destMax);
    if (buffer == NULL) {
        return;
    }
    switch (level) {
        case LEVEL_DEBUG:
            iret = sprintf_s(buffer, destMax, "[Debug]%s\n", msg);
            if (iret < 0) {
                return;
            }
            break;
        case LEVEL_ERROR:
            iret = sprintf_s(buffer, destMax, "[Error]%s\n", msg);
            if (iret < 0) {
                return;
            }
            break;
        case LEVEL_WARN:
            iret = sprintf_s(buffer, destMax, "[Warn]%s\n", msg);
            if (iret < 0) {
                return;
            }
            break;
        default:
            iret = sprintf_s(buffer, destMax, "[Info]%s\n", msg);
            if (iret < 0) {
                return;
            }
    }
    WriteLogFile(logPath, FILE_MAX_SIZE, buffer, strlen(buffer));
    free(buffer);
}

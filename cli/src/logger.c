/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: ascend-docker-cli日志模块
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "securec.h"
#include "basic.h"
#include "utils.h"

#define FILE_MAX_SIZE (1024 * 1024 * 10)
#define LOG_PATH_DIR "/var/log/ascend-docker-runtime/"
#define TEMP_BUFFER 30
#define YEAR_OFFSET 1900
#define MONTH_OFFSET 1
#define LOG_LENGTH 1024

int GetCurrentLocalTime(char* buffer, int length)
{
    time_t rawtime;
    struct tm* timeinfo = NULL;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        return -1;
    }
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

int CreateLog(const char* filename)
{
    int exist;
    exist = access(filename, 0);
    if (exist != 0) {
        return creat(filename, DEFAULT_LOG_MODE);
    }
    return 0;
}

long GetLogSize(const char* filename)
{
    int ret;
    ret = CreateLog(filename);
    if (ret < 0) {
        return -1;
    }
    FILE *fp = NULL;
    char path[PATH_MAX + 1] = {0x00};
    if (strlen(filename) > PATH_MAX || NULL == realpath(filename, path)) {
        return -1;
    }
    fp = fopen(path, "rb");
    long length = 0;
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
    }
    if (NULL != fp) {
        fclose(fp);
        fp = NULL;
    }
    return length;
}


int LogLoop(const char* filename)
{
    int ret;
    char* loopPath = LOG_PATH_DIR"docker-runtime-log.log.1";
    int exist;
    exist = access(loopPath, 0);
    if (exist == 0) {
        unlink(loopPath);
    }
    rename(filename, loopPath);
    ret = CreateLog(filename);
    if (ret < 0) {
        return -1;
    }
    return ret;
}

void WriteLogFile(char* filename, long maxSize, const char* buffer, unsigned bufferSize)
{
    if (filename != NULL && buffer != NULL) {
        char path[PATH_MAX + 1] = {0x00};
        FILE *fp = NULL;
        int ret;
        long length = GetLogSize(filename);
        if (length < 0) {
            return;
        }
        if (length > maxSize) {
            ret = LogLoop(filename);
            if (ret < 0) {
                return;
            }
        }
        if (strlen(filename) > PATH_MAX || NULL == realpath(filename, path)) {
            return;
        }
        fp = fopen(path, "a+");
        if (fp != NULL) {
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

void Logger(const char *msg, int level, int screen)
{
    if (msg == NULL) {
        return;
    }
    if (screen == SCREEN_YES) {
        LOG_ERROR(msg);
    }
    int ret;
    char *logPath = LOG_PATH_DIR"docker-runtime-log.log";
    if (MakeDirWithParent(LOG_PATH_DIR, DEFAULT_LOGDIR_MODE) < 0) {
        return;
    }
    int destMax = LOG_LENGTH;
    if (destMax <= 0) {
        return;
    }
    char* buffer = malloc(destMax);
    if (buffer == NULL) {
        return;
    }
    switch (level) {
        case LEVEL_DEBUG:
            ret = sprintf_s(buffer, destMax, "[Debug]%s\n", msg);
            break;
        case LEVEL_ERROR:
            ret = sprintf_s(buffer, destMax, "[Error]%s\n", msg);
            break;
        case LEVEL_WARN:
            ret = sprintf_s(buffer, destMax, "[Warn]%s\n", msg);
            break;
        default:
            ret = sprintf_s(buffer, destMax, "[Info]%s\n", msg);
    }
    if (ret < 0) {
        free(buffer);
        return;
    }
    WriteLogFile(logPath, FILE_MAX_SIZE, buffer, strlen(buffer));
    free(buffer);
}

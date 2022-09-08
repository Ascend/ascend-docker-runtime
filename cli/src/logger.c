/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: ascend-docker-cli日志模块
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include "securec.h"
#include "basic.h"
#include "utils.h"
#include "logger.h"

#define FILE_MAX_SIZE (1024 * 1024 * 10)
#define LOG_PATH_DIR "/var/log/ascend-docker-runtime/"
#define TEMP_BUFFER 30
#define YEAR_OFFSET 1900
#define MONTH_OFFSET 1
#define LOG_LENGTH 1024

int GetCurrentLocalTime(char* buffer, int length)
{
    if (buffer == NULL) {
        (void)fprintf(stderr, "buffer pointer is null!\n");
        return -1;
    }

    time_t timep = time(NULL);
    if (timep == (time_t)-1) {
        return -1;
    }
    struct tm result = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct tm *timeinfo = localtime_r(&timep, &result);
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
    if (filename == NULL) {
        return -1;
    }
    int exist;
    exist = access(filename, 0);
    int fd = 0;
    if (exist != 0) {
        fd = creat(filename, DEFAULT_LOG_MODE);
        if (fd < 0) {
            return -1;
        }
        close(fd);
    }
    return 0;
}

static long GetLogSizeProcess(const char* path)
{
    FILE *fp = NULL;
    fp = fopen(path, "rb");
    long length = 0;
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

long GetLogSize(const char* filename)
{
    if (filename == NULL) {
        (void)fprintf(stderr, "filename pointer is null!\n");
        return -1;
    }

    int ret;
    ret = CreateLog(filename);
    if (ret < 0) {
        return -1;
    }

    char path[PATH_MAX + 1] = {0x00};
    if (strlen(filename) > PATH_MAX || realpath(filename, path) == NULL) {
        return -1;
    }
    if (!CheckExistsFile(path, strlen(path), 0, false)) {
        return -1;
    }
    return GetLogSizeProcess(path);
}

int LogLoop(const char* filename)
{
    if (filename == NULL) {
        (void)fprintf(stderr, "filename pointer is null!\n");
        return -1;
    }

    int ret;
    char* loopPath = LOG_PATH_DIR"docker-runtime-log.log.1";

    if (!CheckExistsFile(loopPath, strlen(loopPath), 0, false)) {
        return -1;
    }
    if (!CheckExistsFile(filename, strlen(filename), 0, false)) {
        return -1;
    }
    if (access(loopPath, 0) == 0) {
        unlink(loopPath);
    }
    if (rename(filename, loopPath) == -1) {
        return -1;
    }
    if (chmod(loopPath, DUMP_LOG_MODE) != 0) {
        return -1;
    }
    ret = CreateLog(filename);
    if (ret < 0) {
        return -1;
    }
    return ret;
}

static void WriteLogInfo(const char* path, size_t pathLen, const char* buffer, const unsigned bufferSize)
{
    if (path == NULL) {
        return;
    }
    FILE *fp = NULL;
    int ret = 0;
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
    return;
}

static bool LogConvertStorage(const char* filename, const long maxSize)
{
    long length = GetLogSize(filename);
    if (length < 0) {
        return false;
    }
    if (length > maxSize) {
        int ret = LogLoop(filename);
        if (ret < 0) {
            return false;
        }
    }
    return true;
}

static void LogFileProcess(const char* filename, const long maxSize, const char* buffer, const unsigned bufferSize)
{
    if (filename == NULL) {
        return;
    }
    int ret = 0;
    char path[PATH_MAX + 1] = {0x00};
    if (!LogConvertStorage(filename, maxSize)) {
        return;
    }
    if (strlen(filename) > PATH_MAX || realpath(filename, path) == NULL) {
        return;
    }
    if (!CheckExistsFile(path, strlen(path), 0, false)) {
        return;
    }
    WriteLogInfo(path, PATH_MAX + 1, buffer, bufferSize);
}

void WriteLogFile(const char* filename, long maxSize, const char* buffer, unsigned bufferSize)
{
    if (filename == NULL || buffer == NULL) {
        (void)fprintf(stderr, "filename, buffer pointer is null!\n");
        return;
    }
    
    LogFileProcess(filename, maxSize, buffer, bufferSize);
}

static void DivertAndWrite(const char *logPath, const char *msg, const int level)
{
    int ret;
    size_t destMax = LOG_LENGTH;
    if (destMax <= 0) {
        return;
    }
    char* buffer = (char*)malloc(destMax * sizeof(char));
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

    DivertAndWrite(logPath, msg, level);
}

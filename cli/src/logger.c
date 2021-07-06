/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli日志模块
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define FILE_MAX_SIZE (1024*1024*10)
#define LOG_PATH_DIR "/var/log/"
#define TEMP_BUFFER 30

void GetCurrentLocalTime(char* buffer, int length)
{
    time_t rawtime;
    struct tm* timeinfo = NULL;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf_s(buffer, TEMP_BUFFER, "[%04d-%02d-%02d %02d:%02d:%02d]", (timeinfo->tm_year+1900), (timeinfo->tm_mon+1),
    (timeinfo->tm_mday), (timeinfo->tm_hour), (timeinfo->tm_min), (timeinfo->tm_sec));
}

long GetLogSize(char* filename)
{
    long length = 0;
    FILE *fp = NULL;
    fp = fopen(filename, "rb");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
    }
    if (fp != NULL)
    {
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
    if (exist == 0) 
    {
        unlink(loopPath);
    }
    rename(filename, loopPath);
} 

void WriteLogFile(char* filename, long maxSize, char* buffer, unsigned bufferSize)
{
    if (filename != NULL && buffer != NULL) 
    {
        long length = GetLogSize(filename);
        if (length > maxSize)
        {
            LogLoop(filename); 
        }
        FILE *fp;
        fp = fopen(filename, "a+");
        if (fp != NULL)
        {
            char now[TEMP_BUFFER] = {0};
            GetCurrentLocalTime(now, sizeof(now)/sizeof(now[0]));
            fwrite(now, strlen(now), 1, fp);
            fwrite(buffer, bufferSize, 1, fp);
            fclose(fp);
            fp = NULL;
        }
    }
}

void Logger(const char *msg, int level, int length) {
    enum LEVEL { Info=0,  Warn, Error, Debug};
    enum LEVEL _level;
    char *logPath = LOG_PATH_DIR"docker-runtime-log.log";
    _level = level;
    char* buffer = malloc(length+20);
    switch (_level) 
    {
        case Debug:sprintf(buffer, "[Debug]%s\n", msg);
            break;
        case Error:sprintf(buffer, "[Error]%s\n", msg);
            break;
        case Warn:sprintf(buffer, "[Warn]%s\n", msg); 
            break;
        default:sprintf(buffer, "[Info]%s\n", msg);
    }
    WriteLogFile(logPath, FILE_MAX_SIZE, buffer, strlen(buffer));
    free(buffer);
}

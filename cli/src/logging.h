/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具日志模块头文件
*/
#ifndef _LOGGING_H
#define _LOGGING_H

void SetPidForLog(int pid);
int OpenLog(const char *logFile);
void CloseLog();

void LogError(const char *fmt, ...);
void LogInfo(const char *fmt, ...);
void LogWarning(const char *fmt, ...);

#endif

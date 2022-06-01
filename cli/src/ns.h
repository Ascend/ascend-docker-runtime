/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器Namespace实用函数模块头文件
*/
#ifndef _NS_H
#define _NS_H

#include <sys/types.h>

int GetNsPath(int pid, const char *nsType, char *buf, const size_t bufSize);
int GetSelfNsPath(const char *nsType, char *buf, const size_t bufSize);
int EnterNsByFd(int fd, int nsType);
int EnterNsByPath(const char *path, int nsType);

#endif
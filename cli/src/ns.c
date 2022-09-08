/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器Namespace实用函数模块
*/
#define _GNU_SOURCE
#include "ns.h"

#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include "basic.h"
#include "securec.h"
#include "utils.h"
#include "logger.h"

int GetNsPath(const long pid, const char *nsType, char *buf, const size_t bufSize)
{
    if ((nsType == NULL) || (buf == NULL)) {
        return -1;
    }
    static const char *fmtStr = "/proc/%d/ns/%s";
    return sprintf_s(buf, bufSize, fmtStr, pid, nsType);
}

int GetSelfNsPath(const char *nsType, char *buf, const size_t bufSize)
{
    if ((nsType == NULL) || (buf == NULL)) {
        return -1;
    }
    static const char *fmtStr = "/proc/self/ns/%s";
    return sprintf_s(buf, bufSize, fmtStr, nsType);
}

int EnterNsByFd(int fd, int nsType)
{
    int ret = setns(fd, nsType);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to set ns: fd(%d).", fd);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }
    return 0;
}

int EnterNsByPath(const char *path, int nsType)
{
    if (path == NULL) {
        return -1;
    }
    int fd;
    int ret;

    fd = open(path, O_RDONLY); // proc文件接口，非外部输入
    if (fd < 0) {
        Logger("Failed to open ns path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = EnterNsByFd(fd, nsType);
    if (ret < 0) {
        Logger("failed to set ns.", LEVEL_ERROR, SCREEN_YES);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
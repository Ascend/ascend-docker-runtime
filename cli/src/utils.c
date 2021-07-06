/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具实用函数模块
*/
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "securec.h"
#include "logger.h"
char *FormatMessage(char *format, ...){
    va_list list;
    // 获取格式化后字符串的长度
    va_start(list, format);
    int size = vsnprintf(NULL, 0, format, list);
    va_end(list);
    if(size <= 0){
        return NULL;
    }
    size++;
    // 复位va_list, 将格式化字符串写入到buf
    va_start(list, format);
    char *buf = (char *)malloc(size);
    vsnprintf(buf, size, format, list);
    va_end(list);
    return buf;
}

int IsStrEqual(const char *s1, const char *s2)
{
    return (!strcmp(s1, s2));
}

int StrHasPrefix(const char *str, const char *prefix)
{
    return (!strncmp(str, prefix, strlen(prefix)));
}

int MkDir(const char *dir, int mode)
{
    return mkdir(dir, mode);
}

int VerifyPathInfo(const struct PathInfo* pathInfo)
{
    if (pathInfo == NULL || pathInfo->dst == NULL || pathInfo->src == NULL) {
        return -1;
    }
    return 0;
}

int CheckDirExists(const char *dir)
{
    DIR *ptr = opendir(dir);
    if (NULL == ptr) {
        return -1;
    }

    closedir(ptr);
    return 0;
}

int GetParentPathStr(const char *path, char *parent, size_t bufSize)
{
    char *ptr = strrchr(path, '/');
    if (ptr == NULL) {
        return 0;
    }

    int len = (int)strlen(path) - (int)strlen(ptr);
    if (len < 1) {
        return 0;
    }

    errno_t ret = strncpy_s(parent, bufSize, path, len);
    if (ret != EOK) {
        return -1;
    }

    return 0;
}

int MakeDirWithParent(const char *path, mode_t mode)
{
    if (*path == '\0' || *path == '.') {
        return 0;
    }
    if (CheckDirExists(path) == 0) {
        return 0;
    }

    char parentPath[BUF_SIZE] = {0};
    GetParentPathStr(path, parentPath, BUF_SIZE);
    if (strlen(parentPath) > 0 && MakeDirWithParent(parentPath, mode) < 0) {
        return -1;
    }

    int ret = MkDir(path, mode);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

int MakeMountPoints(const char *path, mode_t mode)
{
    /* directory */
    char parentDir[BUF_SIZE] = {0};
    GetParentPathStr(path, parentDir, BUF_SIZE);

    int ret = MakeDirWithParent(parentDir, DEFAULT_DIR_MODE);
    if (ret < 0) {
        Logger(FormatMessage("failed to make parent dir for file: %s", path), 2);
        return -1;
    }

    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path, resolvedPath) == NULL && errno != ENOENT) {
        Logger(FormatMessage("failed to resolve path %s.", path), 2);
        return -1;
    }

    int fd = open(resolvedPath, O_NOFOLLOW | O_CREAT, mode);
    if (fd < 0) {
        Logger(FormatMessage("cannot create file: %s.", resolvedPath), 2);
        return -1;
    }
    close(fd);
    return 0;
}
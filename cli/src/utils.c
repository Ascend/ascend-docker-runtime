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
#include <libgen.h>
#include "securec.h"
#include "logger.h"

char *FormatLogMessage(char *format, ...)
{
    if (format == NULL) {
        fprintf(stderr, "format pointer is null!\n");
        return NULL;
    }

    va_list list;
    // 获取格式化后字符串的长度
    va_start(list, format);
    char buff[1024] = {0};
    int size = vsnprintf_s(buff, sizeof(buff), sizeof(buff) - 1, format, list);
    va_end(list);
    if (size <= 0) {
        return NULL;
    }
    size++;
    // 复位va_list, 将格式化字符串写入到buf
    va_start(list, format);
    char *buf = (char *)malloc(size);
    if (buf == NULL) {
        return NULL;
    }
    int ret = vsnprintf_s(buf, size, size - 1, format, list);
    va_end(list);
    if (ret <= 0) {
        free(buf);
        return NULL;
    }
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
    if (dir == NULL) {
        fprintf(stderr, "dir pointer is null!\n");
        return -1;
    }

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
    if (dir == NULL) {
        fprintf(stderr, "dir pointer is null!\n");
        return -1;
    }

    DIR *ptr = opendir(dir);
    if (NULL == ptr) {
        return -1;
    }

    closedir(ptr);
    return 0;
}

int GetParentPathStr(const char *path, char *parent, size_t bufSize)
{
    if (path == NULL || parent == NULL) {
        fprintf(stderr, "path pointer or parentPath is null!\n");
        return -1;
    }

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
    if (path == NULL) {
        fprintf(stderr, "path pointer is null!\n");
        return -1;
    }

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
    if (path == NULL) {
        fprintf(stderr, "path pointer is null!\n");
        return -1;
    }

    /* directory */
    char parentDir[BUF_SIZE] = {0};
    GetParentPathStr(path, parentDir, BUF_SIZE);

    int ret = MakeDirWithParent(parentDir, DEFAULT_DIR_MODE);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to make parent dir for file: %s", path);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path, resolvedPath) == NULL && errno != ENOENT) {
        char* str = FormatLogMessage("failed to resolve path %s.", path);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    int fd = open(resolvedPath, O_NOFOLLOW | O_CREAT, mode);
    if (fd < 0) {
        char* str = FormatLogMessage("cannot create file: %s.", resolvedPath);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }
    close(fd);
    return 0;
}

int CheckLegality(const char* filename)
{
    if (filename == NULL) {
        fprintf(stderr, "filename pointer is null!\n");
        return -1;
    }
    
    char buf[PATH_MAX + 1] = {0x00};
    errno_t ret = strncpy_s(buf, PATH_MAX + 1, filename, strlen(filename));
    if (ret != EOK) {
        return -1;
    }
    do {
        struct stat fileStat;
        if (stat(buf, &fileStat) != 0) {
            return -1;
        }
        if ((fileStat.st_uid != ROOT_UID) && (fileStat.st_uid != geteuid())) { // 操作文件owner非root/自己
            fprintf(stderr, "Please check the folder owner!\n");
            return -1;
        }
        if ((fileStat.st_mode & S_IWOTH) != 0) { // 操作文件对other用户可写
            fprintf(stderr, "Please check the write permission!\n");
            return -1;
        }
    } while (strncmp(dirname(buf), "/", strlen(dirname(buf))));
    
    return 0;
}
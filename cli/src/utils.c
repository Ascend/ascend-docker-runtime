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
#include <ctype.h>
#include "securec.h"
#include "logger.h"

static bool g_checkWgroup = true;

char *FormatLogMessage(char *format, ...)
{
    if (format == NULL) {
        (void)fprintf(stderr, "format pointer is null!\n");
        return NULL;
    }

    va_list list;
    // 获取格式化后字符串的长度
    va_start(list, format);
    char buff[1024] = {0};
    int ret = vsnprintf_s(buff, sizeof(buff), sizeof(buff) - 1, format, list);
    va_end(list);
    if (ret <= 0) {
        return NULL;
    }
    size_t size = (size_t)ret;
    size++;
    // 复位va_list, 将格式化字符串写入到buf
    va_start(list, format);
    char *buf = (char *)malloc(size);
    if (buf == NULL) {
        return NULL;
    }
    ret = vsnprintf_s(buf, size, size - 1, format, list);
    va_end(list);
    if (ret <= 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

int IsStrEqual(const char *s1, const char *s2)
{
    return (strcmp(s1, s2) == 0);
}

bool StrHasPrefix(const char *str, const char *prefix)
{
    return (strncmp(str, prefix, strlen(prefix)) == 0);
}

static int MkDir(const char *dir, mode_t mode)
{
    if (dir == NULL) {
        (void)fprintf(stderr, "dir pointer is null!\n");
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
        (void)fprintf(stderr, "dir pointer is null!\n");
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
        (void)fprintf(stderr, "path pointer or parentPath is null!\n");
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
        (void)fprintf(stderr, "path pointer is null!\n");
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
        (void)fprintf(stderr, "path pointer is null!\n");
        return -1;
    }

    /* directory */
    char parentDir[BUF_SIZE] = {0};
    GetParentPathStr(path, parentDir, BUF_SIZE);

    int ret = MakeDirWithParent(parentDir, DEFAULT_DIR_MODE);
    if (ret < 0) {
        Logger("Failed to make parent dir for file.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path, resolvedPath) == NULL && errno != ENOENT) {
        Logger("failed to resolve path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int fd = open(resolvedPath, O_NOFOLLOW | O_CREAT, mode);
    if (fd < 0) {
        Logger("cannot create file.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    close(fd);
    return 0;
}

static bool ShowExceptionInfo(const char* exceptionInfo)
{
    (void)fprintf(stderr, exceptionInfo);
    (void)fprintf(stderr, "\n");
    return false;
}

static bool CheckFileOwner(const struct stat fileStat, const bool checkOwner)
{
    if (checkOwner) {
        if ((fileStat.st_uid != ROOT_UID) && (fileStat.st_uid != geteuid())) { // 操作文件owner非root/自己
            return ShowExceptionInfo("Please check the folder owner!");
        }
    }
    return true;
}

static bool CheckParentDir(const char* filePath, const size_t filePathLen,
    struct stat fileStat, const bool checkOwner)
{
    char buf[PATH_MAX] = {0};
    if (strncpy_s(buf, sizeof(buf), filePath, filePathLen) != EOK) {
        return false;
    }
    for (int iLoop = 0; iLoop < PATH_MAX; iLoop++) {
        if (!CheckFileOwner(fileStat, checkOwner)) {
            return false;
        }
        if ((fileStat.st_mode & S_IWOTH) != 0) { // 操作文件对other用户可写
            return ShowExceptionInfo("Please check the write permission!");
        }
        if (g_checkWgroup && ((fileStat.st_mode & S_IWGRP) != 0)) { // 除日志文件外对group可写
            return ShowExceptionInfo("Please check the write permission!");
        }
        if (S_ISLNK(fileStat.st_mode) != 0) { // 存在软链接
            return ShowExceptionInfo("resolvedPath is symbolic link!");
        }
        if ((strcmp(buf, "/") == 0) || (strstr(buf, "/") == NULL)) {
            break;
        }
        if (strcmp(dirname(buf), ".") == 0) {
            break;
        }
        if (lstat(buf, &fileStat) != 0) {
            return false;
        }
    }
    return true;
}

static bool CheckLegality(const char* filePath, const size_t filePathLen,
    const unsigned long long maxFileSzieMb, const bool checkOwner)
{
    const unsigned long long maxFileSzieB = maxFileSzieMb * 1024 * 1024;
    char buf[PATH_MAX] = {0};
    if (strncpy_s(buf, sizeof(buf), filePath, filePathLen) != EOK) {
        return false;
    }
    struct stat fileStat;
    if ((lstat(buf, &fileStat) != 0) ||
        ((S_ISREG(fileStat.st_mode) == 0) && (S_ISDIR(fileStat.st_mode) == 0))) {
        return ShowExceptionInfo("filePath does not exist or is not a file/dir!");
    }
    if ((maxFileSzieMb > 0) && (fileStat.st_size >= maxFileSzieB)) { // 文件大小超限，日志文件不校验大小，由轮滚机制保护
        return ShowExceptionInfo("fileSize out of bounds!");
    }
    return CheckParentDir(filePath, filePathLen, fileStat, checkOwner);
}

bool IsValidChar(const char c)
{
    if (isalnum(c) != 0) {
        return true;
    }
    // ._-/~为合法字符
    if ((c == '.') || (c == '_') ||
        (c == '-') || (c == '/') || (c == '~')) {
        return true;
    }
    return false;
}

bool CheckExternalFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkOwner)
{
    if ((filePathLen > PATH_MAX) || (filePathLen <= 0)) { // 长度越界
        return ShowExceptionInfo("filePathLen out of bounds!");
    }
    for (size_t iLoop = 0; iLoop < filePathLen; iLoop++) {
        if (!IsValidChar(filePath[iLoop])) { // 非法字符
            return ShowExceptionInfo("filePath has an illegal character!");
        }
    }
    return CheckLegality(filePath, filePathLen, maxFileSzieMb, checkOwner);
}

bool CheckExistsFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkWgroup)
{
    struct stat fileStat;
    if (lstat(filePath, &fileStat) != 0) {
        return true; // 文件不存在
    }
    if (S_ISREG(fileStat.st_mode) == 0) { // 不是文件
        return false;
    }
    g_checkWgroup = checkWgroup;
    if (!CheckExternalFile(filePath, filePathLen, maxFileSzieMb, true)) {
        g_checkWgroup = true;
        return false;
    }
    g_checkWgroup = true;
    return true;
}

static bool CheckFileSubset(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb)
{
    const unsigned long long maxFileSzieB = maxFileSzieMb * 1024 * 1024;
    int iLoop;
    if ((filePathLen > PATH_MAX) || (filePathLen <= 0)) { // 长度越界
        return ShowExceptionInfo("filePathLen out of bounds!");
    }
    for (iLoop = 0; iLoop < filePathLen; iLoop++) {
        if (!IsValidChar(filePath[iLoop])) { // 非法字符
            return ShowExceptionInfo("filePath has an illegal character!");
        }
    }
    struct stat fileStat;
    if (lstat(filePath, &fileStat) != 0) {
        return ShowExceptionInfo("filePath does not exist!");
    }
    if (S_ISLNK(fileStat.st_mode) != 0) { // 存在软链接
        return ShowExceptionInfo("filePath is symbolic link!");
    }
    if (fileStat.st_size >= maxFileSzieB) { // 文件大小超限
        return ShowExceptionInfo("fileSize out of bounds!");
    }
    return true;
}

bool GetFileSubsetAndCheck(const char *basePath, const size_t basePathLen)
{
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    char base[PATH_MAX] = {0};
 
    if ((dir = opendir(basePath)) == NULL) {
        return ShowExceptionInfo("Open dir error!");
    }
    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        memset_s(base, PATH_MAX, 0, PATH_MAX);
        if (strcpy_s(base, PATH_MAX, basePath) != 0) {
            return ShowExceptionInfo("Strcpy failed!");
        }
        if (strcat_sp(base, PATH_MAX, "/") != 0 ||
            strcat_sp(base, PATH_MAX, ptr->d_name) != 0) {
            return ShowExceptionInfo("Strcat failed!");
        }
        if (ptr->d_type == DT_REG) { // 文件
            const size_t maxFileSzieMb = 10; // max 10 MB
            if (!(base, strlen(base), maxFileSzieMb)) {
                return false;
            }
        } else if (ptr->d_type == DT_LNK) { // 软链接
            return ShowExceptionInfo("FilePath has a soft link!");
        } else if (ptr->d_type == DT_DIR) { // 目录
            if (!GetFileSubsetAndCheck(base, strlen(base))) {
                return false;
            }
        }
    }
    return true;
}
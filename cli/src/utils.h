/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具实用函数模块头文件
*/
#ifndef _UTILS_H
#define _UTILS_H

#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <ctype.h>
#include "basic.h"

char *FormatLogMessage(char *format, ...);
int IsStrEqual(const char *s1, const char *s2);
bool StrHasPrefix(const char *str, const char *prefix);
int VerifyPathInfo(const struct PathInfo* pathInfo);
int CheckDirExists(const char *dir);
int GetParentPathStr(const char *path, char *parent, size_t bufSize);
int MakeDirWithParent(const char *path, mode_t mode);
int MakeMountPoints(const char *path, mode_t mode);
bool IsValidChar(const char c);
bool CheckExternalFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkOwner);
bool GetFileSubsetAndCheck(const char *basePath, const size_t basePathLen);
bool CheckExistsFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkWgroup);
#endif
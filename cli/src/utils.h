/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具实用函数模块头文件
*/
#ifndef _UTILS_H
#define _UTILS_H

#include <stdbool.h>
#include <sys/types.h>
#include "basic.h"

// For cgroup setup
int StrHasPrefix(const char *str, const char *prefix);
bool CheckRootDir(char **pLine);
bool CheckFsType(char **pLine);
bool CheckSubStr(char **pLine, const char *subsys);
typedef char *(*ParseFileLine)(char *, const char *);
int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath);

// For mount setup
int MkDir(const char *dir, int mode);
int VerifyPathInfo(const struct PathInfo* pathInfo);
int CheckDirExists(const char *dir);
int GetParentPathStr(const char *path, char *parent, size_t bufSize);
int MakeParentDir(const char *path, mode_t mode);
int CreateFile(const char *path, mode_t mode);
int Mount(const char *src, const char *dst);

#endif
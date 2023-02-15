/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
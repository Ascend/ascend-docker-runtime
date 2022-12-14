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
#ifndef _NS_H
#define _NS_H

#include <sys/types.h>

int GetNsPath(const long pid, const char *nsType, char *buf, const size_t bufSize);
int GetSelfNsPath(const char *nsType, char *buf, const size_t bufSize);
int EnterNsByFd(int fd, int nsType);
int EnterNsByPath(const char *path, int nsType);

#endif
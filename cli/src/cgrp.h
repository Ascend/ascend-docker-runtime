/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器CGroup配置模块头文件
*/
#ifndef _CGRP_H
#define _CGRP_H

#include "basic.h"

int GetCgroupPath(int pid, char *effPath, size_t maxSize);
int SetupCgroup(const struct ParsedConfig *config);

#endif
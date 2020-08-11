/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具挂载选项头文件
*/
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include <stdbool.h>

int ParseRuntimeOptions(const char *options);
bool IsOptionNoDrvSet();
bool IsOptionVerboseSet();

#endif

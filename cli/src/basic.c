/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#include "basic.h"

void InitParsedConfig(struct ParsedConfig *parsedConfig)
{
    if (parsedConfig != NULL)
    {
        Logger("parsedConfig Pointer is null!.");
        return;
    }
    parsedConfig->devicesNr = MAX_DEVICE_NR;
}
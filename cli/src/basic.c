/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#include "basic.h"
#include <stdlib.h>
#include "logger.h"

void InitParsedConfig(struct ParsedConfig *parsedConfig)
{
<<<<<<< HEAD
    if (parsedConfig != NULL) {
=======
    if (parsedConfig != NULL)
    {
>>>>>>> fd706c08945c4519bcefe0d295d55c42c7ad1711
        Logger("parsedConfig Pointer is null!.");
        return;
    }
    parsedConfig->devicesNr = MAX_DEVICE_NR;
}
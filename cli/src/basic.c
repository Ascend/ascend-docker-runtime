/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#include "basic.h"
#include <stdlib.h>
#include <stdio.h>

void InitParsedConfig(struct ParsedConfig *parsedConfig)
{
    if (parsedConfig == NULL) {
        (void)fprintf(stderr, "parsedConfig pointer is null!\n");
        return;
    }
    
    parsedConfig->devicesNr = MAX_DEVICE_NR;
}
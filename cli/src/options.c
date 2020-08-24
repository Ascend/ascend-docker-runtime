/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具挂载选项
*/
#include "options.h"

#include <string.h>
#include <stdlib.h>
#include "securec.h"

static struct {
    bool noDrv;
    bool verbose;
} g_runtimeOptions;

static struct {
    const char *name;
    bool *flag;
} g_optionNameFlagTable[] = {
    {"NODRV", &g_runtimeOptions.noDrv}, // 不挂载Driver
    {"VERBOSE", &g_runtimeOptions.verbose}, // 输出日志
    {NULL, NULL}
};

void ParseRuntimeOptions(const char *options)
{
    // set defaults value
    g_runtimeOptions.noDrv = false;
    g_runtimeOptions.verbose = false;

    static const char *seperator = ",";
    char *runtimeOptions = strdup(options);
    char *context = NULL;
    char *token = NULL;

    token = strtok_s(runtimeOptions, seperator, &context);
    while (token != NULL) {
        for (int i = 0; g_optionNameFlagTable[i].name != NULL; i++) {
            if (strcmp((const char *)token, g_optionNameFlagTable[i].name)) {
                continue;
            }

            *g_optionNameFlagTable[i].flag = true;
        }

        token = strtok_s(NULL, seperator, &context);
    }

    free(runtimeOptions);
}

bool IsOptionNoDrvSet()
{
    return g_runtimeOptions.noDrv;
}

bool IsOptionVerboseSet()
{
    return g_runtimeOptions.verbose;
}
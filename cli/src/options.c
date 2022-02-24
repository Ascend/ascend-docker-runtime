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
    bool isVirtual;
} g_runtimeOptions;

static struct {
    const char *name;
    bool *flag;
} g_optionNameFlagTable[] = {
    {"NODRV", &g_runtimeOptions.noDrv}, // 不挂载Driver
    {"VIRTUAL", &g_runtimeOptions.isVirtual},
    {NULL, NULL}
};

void ParseRuntimeOptions(const char *options)
{
    if (options == NULL) {
        fprintf(stderr, "options pointer is null!\n");
        return;
    }

    // set defaults value
    g_runtimeOptions.noDrv = false;
    g_runtimeOptions.isVirtual = false;

    static const char *seperator = ",";
    char *runtimeOptions = strdup(options);
    char *context = NULL;
    char *token = NULL;

    for (token = strtok_s(runtimeOptions, seperator, &context);
    token != NULL;
    token = strtok_s(NULL, seperator, &context)) {
        for (int i = 0; g_optionNameFlagTable[i].name != NULL; i++) {
            if (!strcmp((const char *)token, g_optionNameFlagTable[i].name)) {
                *g_optionNameFlagTable[i].flag = true;
            }
        }
    }

    free(runtimeOptions);
}

bool IsOptionNoDrvSet()
{
    return g_runtimeOptions.noDrv;
}

bool IsVirtual()
{
    return g_runtimeOptions.isVirtual;
}

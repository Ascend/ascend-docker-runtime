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
        (void)fprintf(stderr, "options pointer is null!\n");
        return;
    }

    // set defaults value
    g_runtimeOptions.noDrv = false;
    g_runtimeOptions.isVirtual = false;

    static const char *seperator = ",";
    char *runtimeOptions = strdup(options);
    if (runtimeOptions == NULL) {
        (void)fprintf(stderr, "strdup failed!\n");
        return;
    }
    char *context = NULL;
    char *token = NULL;

    for (token = strtok_s(runtimeOptions, seperator, &context);
    token != NULL;
    token = strtok_s(NULL, seperator, &context)) {
        for (int i = 0; g_optionNameFlagTable[i].name != NULL; i++) {
            if (strcmp((const char *)token, g_optionNameFlagTable[i].name) == 0) {
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

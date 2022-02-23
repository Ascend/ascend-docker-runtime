/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend_docker_install工具，用于辅助用户安装ascend_docker
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cJSON.h"

#define MAX_JSON_FILE_SIZE 65535
#define MIN_ARGS_NUM 4
#define ADD_CMD_ARGS_NUM 5
#define ADD_CMD "add"
#define RM_CMD "rm"
#define CMD_INDEX 1
#define FINAL_FILE_INDEX 2
#define TEMP_FILE_INDEX 3
#define RUNTIME_PATH_INDEX 4
#define ASCEND_RUNTIME_PATH_KEY "path"
#define ASCEND_RUNTIME_ARGS_KEY "runtimeArgs"
#define RUNTIME_KEY "runtimes"
#define ASCEND_RUNTIME_NAME "ascend"
#define DEFALUT_KEY "default-runtime"
#define DEFAULT_VALUE "ascend"

static void ReadJsonFile(FILE *pf, char *text, int maxBufferSize)
{
    if (pf == NULL || text == NULL) {
        fprintf(stderr, "file pointer or text pointer are null!\n");
        return;
    }

    (void)fseek(pf, 0, SEEK_END);

    int size = (int)ftell(pf);
    if (size >= maxBufferSize) {
        fprintf(stderr, "file size too large\n");
        return;
    }

    (void)fseek(pf, 0, SEEK_SET);
    if (fread(text, sizeof(char), size, pf) != 0) {
        return;
    }
    text[size] = '\0';
}

static cJSON *CreateAscendRuntimeInfo(const char *runtimePath)
{
    if (runtimePath == NULL) {
        fprintf(stderr, "runtimePath pointer are null!\n");
        return NULL;
    }

    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if (root == NULL) {
        fprintf(stderr, "create ascend runtime info root err\n");
        return NULL;
    }

    cJSON *newString = NULL;
    newString = cJSON_CreateString(runtimePath);
    if (newString == NULL) {
        fprintf(stderr, "create ascend runtime info path value err\n");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON *paraArray = NULL;
    paraArray = cJSON_CreateArray();
    if (paraArray == NULL) {
        fprintf(stderr, "create ascend runtime info args err\n");
        cJSON_Delete(root);
        cJSON_Delete(newString);
        return NULL;
    }

    cJSON_AddItemToObject(root, ASCEND_RUNTIME_PATH_KEY, newString);
    cJSON_AddItemToObject(root, ASCEND_RUNTIME_ARGS_KEY, paraArray);

    return root;
}

static cJSON *CreateRuntimes(const char *runtimePath)
{
    if (runtimePath == NULL) {
        fprintf(stderr, "runtimePath pointer is null!\n");
        return NULL;
    }

    cJSON *ascendRuntime = NULL;
    ascendRuntime = CreateAscendRuntimeInfo(runtimePath);
    if (ascendRuntime == NULL) {
        fprintf(stderr, "create ascendruntime err\n");
        return NULL;
    }

    cJSON *runtimes = NULL;
    runtimes = cJSON_CreateObject();
    if (runtimes == NULL) {
        fprintf(stderr, "create runtimes err\n");
        cJSON_Delete(ascendRuntime);
        return NULL;
    }

    cJSON_AddItemToObject(runtimes, ASCEND_RUNTIME_NAME, ascendRuntime);

    return runtimes;
}

static int DelJsonContent(cJSON *root, const char *key)
{
    if (root == NULL || key == NULL) {
        fprintf(stderr, "userInfo  pointer is null!\n");
        return -1;
    }

    cJSON *existItem = NULL;
    existItem = cJSON_GetObjectItem(root, key);
    if (existItem == NULL) {
        return 0;
    }

    cJSON *removedItem = NULL;
    removedItem = cJSON_DetachItemViaPointer(root, existItem);
    if (removedItem == NULL) {
        fprintf(stderr, "remove %s failed\n", key);
        return -1;
    }

    cJSON_Delete(removedItem);
    return 0;
}

static cJSON *CreateContent(const char *runtimePath)
{
    if (runtimePath == NULL) {
        fprintf(stderr, "runtimePath pointer is null!\n");
        return NULL;
    }

    /* 插入ascend runtime */
    cJSON *runtimes = NULL;
    runtimes = CreateRuntimes(runtimePath);
    if (runtimes == NULL) {
        fprintf(stderr, "create runtimes err\n");
        return NULL;
    }

    cJSON *defaultRuntime = NULL;
    defaultRuntime = cJSON_CreateString(DEFAULT_VALUE);
    if (defaultRuntime == NULL) {
        cJSON_Delete(runtimes);
        return NULL;
    }

    cJSON *root = NULL;
    root = cJSON_CreateObject();
    if (root == NULL) {
        /* ascendRuntime已经挂载到runtimes上了，再释放会coredump */
        fprintf(stderr, "create root err\n");
        cJSON_Delete(runtimes);
        cJSON_Delete(defaultRuntime);
        return NULL;
    }

    cJSON_AddItemToObject(root, RUNTIME_KEY, runtimes);

    cJSON_AddItemToObject(root, DEFALUT_KEY, defaultRuntime);

    return root;
}

static cJSON *ModifyContent(FILE *pf, const char *runtimePath)
{
    if (pf == NULL || runtimePath == NULL) {
        fprintf(stderr, "file pointer or runtimePath pointer is null!\n");
        return NULL;
    }

    char jsonStr[MAX_JSON_FILE_SIZE] = {0x0};
    ReadJsonFile(pf, &jsonStr[0], MAX_JSON_FILE_SIZE);

    cJSON *root = NULL;
    root = cJSON_Parse(jsonStr);
    if (root == NULL) {
        fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }

    /* 插入ascend runtime */
    cJSON *runtimes = NULL;
    runtimes = cJSON_GetObjectItem(root, "runtimes");
    if (runtimes == NULL) {
        runtimes = CreateRuntimes(runtimePath);
        if (runtimes == NULL) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToObject(root, RUNTIME_KEY, runtimes);
    } else {
        int ret = DelJsonContent(runtimes, ASCEND_RUNTIME_NAME);
        if (ret != 0) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON  *ascendRuntime = NULL;
        ascendRuntime = CreateAscendRuntimeInfo(runtimePath);
        if (ascendRuntime == NULL) {
            cJSON_Delete(root);
            return NULL;
        }
        cJSON_AddItemToObject(runtimes, ASCEND_RUNTIME_NAME, ascendRuntime);
    }

    /* 插入defaul runtime */
    int ret = DelJsonContent(root, DEFALUT_KEY);
    if (ret != 0) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON *defaultRuntime = cJSON_CreateString(DEFAULT_VALUE);
    if (defaultRuntime == NULL) {
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, DEFALUT_KEY, defaultRuntime);

    return root;
}

static cJSON *RemoveContent(FILE *pf)
{
    if (pf == NULL) {
        fprintf(stderr, "file pointer is null!\n");
        return NULL;
    }

    char jsonStr[MAX_JSON_FILE_SIZE] = {0x0};
    ReadJsonFile(pf, &jsonStr[0], MAX_JSON_FILE_SIZE);

    cJSON *root = NULL;
    root = cJSON_Parse(jsonStr);
    if (root == NULL) {
        fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }

    /* 去除default runtimes */
    int ret = DelJsonContent(root, DEFALUT_KEY);
    if (ret != 0) {
        cJSON_Delete(root);
        return NULL;
    }

    /* 去除runtimes */
    cJSON *runtimes = NULL;
    runtimes = cJSON_GetObjectItem(root, RUNTIME_KEY);
    if (runtimes == NULL) {
        fprintf(stderr, "no runtime key found\n");
        cJSON_Delete(root);
        return NULL;
    }

    ret = DelJsonContent(runtimes, DEFAULT_VALUE);
    if (ret != 0) {
        cJSON_Delete(root);
        return NULL;
    }

    return root;
}


static int DetectAndCreateJsonFile(const char *filePath, const char *tempPath, const char *runtimePath)
{
    if (filePath == NULL || tempPath == NULL || runtimePath == NULL) {
        fprintf(stderr, "filePath, tempPath or runtimePath are null!\n");
        return -1;
    }
    
    cJSON *root = NULL;
    FILE *pf = NULL;
    pf = fopen(filePath, "r+");
    if (pf == NULL) {
        root = CreateContent(runtimePath);
    } else {
        root = ModifyContent(pf, runtimePath);
        fclose(pf);
    }

    if (root == NULL) {
        fprintf(stderr, "error: failed to create json\n");
        return -1;
    }

    pf = fopen(tempPath, "w");
    if (pf == NULL) {
        fprintf(stderr, "error: failed to create file\n");
        return -1;
    }

    if (fprintf(pf, "%s", cJSON_Print(root)) < 0) {
        fprintf(stderr, "error: failed to create file\n");
        (void)fclose(pf);
        cJSON_Delete(root);
        return -1;
    }
    (void)fclose(pf);

    cJSON_Delete(root);

    return 0;
}

static int CreateRevisedJsonFile(const char *filePath, const char *tempPath)
{
    if (filePath == NULL || tempPath == NULL) {
        fprintf(stderr, "filePath or tempPath are null!\n");
        return -1;
    }

    FILE *pf = NULL;
    pf = fopen(filePath, "r+");
    if (pf == NULL) {
        fprintf(stderr, "error: no json files found\n");
        return -1;
    }
    cJSON *newContent = NULL;
    newContent = RemoveContent(pf);
    (void)fclose(pf);

    if (newContent == NULL) {
        fprintf(stderr, "error: failed to create json\n");
        return -1;
    }

    pf = fopen(tempPath, "w");
    if (pf == NULL) {
        fprintf(stderr, "error: failed to create file\n");
        cJSON_Delete(newContent);
        return -1;
    }

    if (fprintf(pf, "%s", cJSON_Print(newContent)) < 0) {
        fprintf(stderr, "error: failed to create file\n");
        cJSON_Delete(newContent);
        return -1;
    }
    (void)fclose(pf);

    cJSON_Delete(newContent);

    return 0;
}

/* 该函数只负责生成json.bak文件，由调用者进行覆盖操作 */
int main(int argc, char *argv[])
{
    if (argc < MIN_ARGS_NUM) {
        return -1;
    }

    printf("%s\n", argv[FINAL_FILE_INDEX]);
    printf("%s\n", argv[TEMP_FILE_INDEX]);
    printf("%s\n", argv[CMD_INDEX]);

    if (strcmp(argv[CMD_INDEX], ADD_CMD) == 0) {
        if (argc != ADD_CMD_ARGS_NUM) {
            return -1;
        }

        return DetectAndCreateJsonFile(argv[FINAL_FILE_INDEX], argv[TEMP_FILE_INDEX], argv[RUNTIME_PATH_INDEX]);
    }

    return CreateRevisedJsonFile(argv[FINAL_FILE_INDEX], argv[TEMP_FILE_INDEX]);
}

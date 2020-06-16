/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend_docker_install工具，用于辅助用户安装ascend_docker
*/
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_JSON_FILE_SIZE 65535
#define NUM_ARGS 4
#define ADD_CMD "add"
#define RM_CMD "rm"
#define CMD_INDEX 1
#define FINAL_FILE_INDEX 2
#define TEMP_FILE_INDEX 3
#define ASCEND_RUNTIME_PATH_VALUE "/usr/bin/ascend-docker-runtime"
#define ASCEND_RUNTIME_PATH_KEY "path"
#define ASCEND_RUNTIME_ARGS_KEY "runtimeArgs"
#define RUNTIME_KEY "runtimes"
#define ASCEND_RUNTIME_NAME "ascend"
#define DEFALUT_KEY "default-runtime"
#define DEFAULT_VALUE "ascend"

void ReadJsonFile(FILE *pf, char *text, int maxBufferSize)
{
    fseek(pf, 0, SEEK_END);

    int size = (int)ftell(pf);
    if (size >= maxBufferSize) {
        fprintf(stderr, "file size too large\n");
        return;
    }
    
    fseek(pf, 0, SEEK_SET);
    fread(text, sizeof(char), size, pf);
    text[size] = '\0';
}

cJSON *CreateAscendRuntimeInfo()
{
    cJSON *root = NULL; 
    root = cJSON_CreateObject();
    if (root == NULL) {
        fprintf(stderr, "create ascend runtime info root err\n");
        return NULL;
    }
    
    cJSON *newString = NULL; 
    newString = cJSON_CreateString(ASCEND_RUNTIME_PATH_VALUE);
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

cJSON *CreateRuntimes()
{
    cJSON *ascendRuntime = NULL;
    ascendRuntime = CreateAscendRuntimeInfo(); 
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

int DelJsonContent(cJSON *root, const char *key)
{
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

cJSON *CreateContent()
{   
    /* 插入ascend runtime */
    cJSON *runtimes = NULL;
    runtimes = CreateRuntimes();
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

cJSON *ModifyContent(FILE *pf)
{
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
        runtimes = CreateRuntimes();
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
        ascendRuntime = CreateAscendRuntimeInfo();
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

cJSON *RemoveContent(FILE *pf)
{
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


int DetectAndCreateJsonFile(const char *filePath, const char *tempPath)
{
    cJSON *root = NULL;
    FILE *pf = NULL; 
    pf = fopen(filePath, "r+");
    if (pf == NULL) {
        root = CreateContent();
    } else {
        root = ModifyContent(pf);
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
    
    fprintf(pf, "%s", cJSON_Print(root));
    fclose(pf);

    cJSON_Delete(root);

    return 0;
}

int CreateRevisedJsonFile(const char *filePath, const char *tempPath)
{
    FILE *pf = NULL; 
    pf = fopen(filePath, "r+");
    if (pf == NULL) {
        fprintf(stderr, "error: no json files found\n");
        return -1;
    }
    cJSON *newContent = NULL;
    newContent = RemoveContent(pf);
    fclose(pf);

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
    
    fprintf(pf, "%s", cJSON_Print(newContent));
    fclose(pf);

    cJSON_Delete(newContent);

    return 0;  
}

/* 该函数只负责生成json.bak文件，由调用者进行覆盖操作 */
int main(int argc, char *argv[])
{
    if (argc != NUM_ARGS) {
        return -1;
    }
    printf("%s\n", argv[FINAL_FILE_INDEX]);
    printf("%s\n", argv[TEMP_FILE_INDEX]);
    printf("%s\n", argv[CMD_INDEX]);
    if (strcmp(argv[CMD_INDEX], ADD_CMD) == 0) {
        return DetectAndCreateJsonFile(argv[FINAL_FILE_INDEX], argv[TEMP_FILE_INDEX]);
    }
    return CreateRevisedJsonFile(argv[FINAL_FILE_INDEX], argv[TEMP_FILE_INDEX]);   
}

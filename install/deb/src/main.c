/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend_docker_install工具，用于辅助用户安装ascend_docker
*/
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_JSON_FILE_SIZE 65535
#define NUM_ARGS 4
#define ADD_CMD "add"
#define RM_CMD "rm"
#define CMD_INDEX 1
#define FINAL_FILE_INDEX 2
#define TEMP_FILE_INDEX 3
#define PATH_VALUE "/usr/local/bin/ascend-docker-runtime"

void ReadJsonFile(const FILE *pf, char *text, int maxBufferSize)
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

int CreateNode(cJSON *runtimes)
{ 
    cJSON *node = cJSON_GetObjectItem(runtimes, "ascend");
    if (node != NULL) {
        return 0;
    }

    cJSON *newItem = NULL;
    newItem = cJSON_CreateObject();
    if (newItem == NULL) {
        return -1;
    }
    cJSON_AddItemToObject(runtimes, "ascend", newItem);

    cJSON *paraArray = NULL;
    paraArray = cJSON_CreateArray();
    if (paraArray == NULL) {
        return -1;
    }
    
    cJSON_AddItemToObject(newItem, "path", cJSON_CreateString(PATH_VALUE));
    cJSON_AddItemToObject(newItem, "runtimeArgs", paraArray);

    return 0;
}

int CreateRuntimes(cJSON *root)
{
    cJSON *runtimes = cJSON_CreateObject();
    if (runtimes == NULL) {
        return -1;
    }
    cJSON_AddItemToObject(root, "runtimes", runtimes);
    return CreateNode(runtimes);
}

cJSON *CreateContent()
{
    cJSON *root = NULL; 
    root = cJSON_CreateObject();
    if (root == NULL) {
        return NULL;
    }
    
    int ret = CreateRuntimes(root);
    if (ret != 0) {
        return NULL;
    }
    
    return root;
}

cJSON *InsertContent(const FILE *pf)
{
    char jsonStr[MAX_JSON_FILE_SIZE] = {0x0};
    ReadJsonFile(pf, &jsonStr[0], MAX_JSON_FILE_SIZE);
    cJSON *root = NULL;
    root = cJSON_Parse(jsonStr);
    if (!root) {
        fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }
    
    cJSON *runtimes = NULL;
    runtimes = cJSON_GetObjectItem(root, "runtimes");
    int ret;
    if (runtimes == NULL) {
        ret = CreateRuntimes(root);
    } else {
        ret = CreateNode(runtimes);
    }
    if (ret != 0) {
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
        root = InsertContent(pf);
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

cJSON *GetNewContent(const FILE *pf)
{
    char jsonStr[MAX_JSON_FILE_SIZE] = {0x0};
    ReadJsonFile(pf, &jsonStr[0], MAX_JSON_FILE_SIZE);

    cJSON *root = NULL;
    root = cJSON_Parse(jsonStr);
    if (!root) {
        fprintf(stderr, "Error before: [%s]\n", cJSON_GetErrorPtr());
        return NULL;
    }
    
    cJSON *runtimes = NULL;
    runtimes = cJSON_GetObjectItem(root, "runtimes");
    if (runtimes == NULL) {
        fprintf(stderr, "no runtime key found\n");
        cJSON_Delete(root);
        return NULL;
    }

    cJSON *ascend = NULL;
    ascend = cJSON_GetObjectItem(runtimes, "ascend");
    if (ascend == NULL) {
        fprintf(stderr, "no ascend key found\n");
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON *removedItem = NULL;
    removedItem = cJSON_DetachItemViaPointer(runtimes, ascend);
    if (removedItem == NULL) {
        fprintf(stderr, "remove runtime failed\n");
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON_Delete(removedItem);
    return root;
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
    newContent = GetNewContent(pf);
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
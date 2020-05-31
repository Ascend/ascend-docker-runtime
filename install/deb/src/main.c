/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend_docker_install工具，用于辅助用户安装ascend_docker
 * auther: z00378930 zhang ou aka snake
*/
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_JSON_FILE_SIZE 65535
#define NUM_ARGS 3
#define FINAL_INDEX 1
#define TENP_INDEX 2

void JsonFileRead(const FILE *pf, char *text, int maxBufferSize)
{
    fseek(pf, 0, SEEK_END);

    int size = (int)ftell(pf);
    if (size >= maxBufferSize) {
        fprintf(stderr, "file size too large\n");
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
    cJSON_AddItemToObject(newItem, "runtimeArges", paraArray);

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
    JsonFileRead(pf, &jsonStr[0], MAX_JSON_FILE_SIZE);
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

/* 该函数只负责生成json.bak文件，由调用者进行覆盖操作 */
int main(int argc, char *argv[])
{
    if (argc != NUM_ARGS) {
        return -1;
    }
    return DetectAndCreateJsonFile(argv[1], argv[2]);    
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: 算力切分数据销毁入口
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <link.h>
#include <dlfcn.h>
#include "securec.h"

#define DCMI_INIT                  "dcmi_init"
#define DCMI_SET_DESTROY_VDEVICE   "dcmi_set_destroy_vdevice"
#define ROOT_UID           0
#define DECIMAL            10
#define DESTROY_PARAMS_NUM 4
#define PARAMS_SECOND      1
#define PARAMS_THIRD       2
#define PARAMS_FOURTH      3
#define ID_MAX             65535

static bool ShowExceptionInfo(const char* exceptionInfo)
{
    (void)fprintf(stderr, exceptionInfo);
    (void)fprintf(stderr, "\n");
    return false;
}
 
static bool CheckLegality(const char* resolvedPath, const size_t resolvedPathLen,
    const unsigned long long maxFileSzieMb, const bool checkOwner)
{
    const unsigned long long maxFileSzieB = maxFileSzieMb * 1024 * 1024;
    char buf[PATH_MAX] = {0};
    if (strncpy_s(buf, sizeof(buf), resolvedPath, resolvedPathLen) != EOK) {
        return false;
    }
    struct stat fileStat;
    if ((stat(buf, &fileStat) != 0) ||
        ((S_ISREG(fileStat.st_mode) == 0) && (S_ISDIR(fileStat.st_mode) == 0))) {
        return ShowExceptionInfo("resolvedPath does not exist or is not a file!");
    }
    if (fileStat.st_size >= maxFileSzieB) { // 文件大小超限
        return ShowExceptionInfo("fileSize out of bounds!");
    }
    for (int iLoop = 0; iLoop < PATH_MAX; iLoop++) {
        if (checkOwner) {
            if ((fileStat.st_uid != ROOT_UID) && (fileStat.st_uid != geteuid())) { // 操作文件owner非root/自己
                return ShowExceptionInfo("Please check the folder owner!");
            }
        }
        if ((fileStat.st_mode & S_IWOTH) != 0) { // 操作文件对other用户可写
            return ShowExceptionInfo("Please check the write permission!");
        }
        if ((strcmp(buf, "/") == 0) || (strstr(buf, "/") == NULL)) {
            break;
        }
        if (strcmp(dirname(buf), ".") == 0) {
            break;
        }
        if (stat(buf, &fileStat) != 0) {
            return false;
        }
    }
    return true;
}
 
static bool CheckExternalFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkOwner)
{
    int iLoop;
    if ((filePathLen > PATH_MAX) || (filePathLen <= 0)) { // 长度越界
        return ShowExceptionInfo("filePathLen out of bounds!");
    }
    if (strstr(filePath, "..") != NULL) { // 存在".."
        return ShowExceptionInfo("filePath has an illegal character!");
    }
    for (iLoop = 0; iLoop < filePathLen; iLoop++) {
        if ((isalnum(filePath[iLoop]) == 0) && (filePath[iLoop] != '.') && (filePath[iLoop] != '_') &&
            (filePath[iLoop] != '-') && (filePath[iLoop] != '/') && (filePath[iLoop] != '~')) { // 非法字符
            return ShowExceptionInfo("filePath has an illegal character!");
        }
    }
    char resolvedPath[PATH_MAX] = {0};
    if (realpath(filePath, resolvedPath) == NULL && errno != ENOENT) {
        return ShowExceptionInfo("realpath failed!");
    }
    if (strstr(resolvedPath, filePath) == NULL) { // 存在软链接
        return ShowExceptionInfo("filePath has a soft link!");
    }
    return CheckLegality(resolvedPath, strlen(resolvedPath), maxFileSzieMb, checkOwner);
}

static bool DeclareDcmiApiAndCheck(void **handle)
{
    *handle = dlopen("libdcmi.so", RTLD_LAZY);
    if (*handle == NULL) {
        (void)fprintf(stderr, "dlopen failed.\n");
        return false;
    }
    char pLinkMap[sizeof(struct link_map)] = {0};
    int ret = dlinfo(*handle, RTLD_DI_LINKMAP, &pLinkMap);
    if (ret == 0) {
        struct link_map* pLink = *(struct link_map**)pLinkMap;
        const size_t maxFileSzieMb = 10; // max 10 mb
        if (!CheckExternalFile(pLink->l_name, strlen(pLink->l_name), maxFileSzieMb, true)) {
            (void)fprintf(stderr, "check sofile failed.\n");
            return false;
        }
    } else {
        (void)fprintf(stderr, "dlinfo sofile failed.\n");
        return false;
    }
    
    return true;
}

static void DcmiDlAbnormalExit(void **handle, const char* errorInfo)
{
    (void)fprintf(stderr, errorInfo);
    (void)fprintf(stderr, "\n");
    if (*handle != NULL) {
        dlclose(*handle);
        *handle = NULL;
    }
}

static void DcmiDlclose(void **handle)
{
    if (*handle != NULL) {
        dlclose(*handle);
        *handle = NULL;
    }
}

static int DestroyEntrance(const char *argv[])
{
    if (argv == NULL) {
        return -1;
    }
    errno = 0;
    int cardId = atoi(argv[PARAMS_SECOND]);
    if ((errno != 0) || (cardId < 0) || (cardId > ID_MAX)) {
        return -1;
    }
    int deviceId = atoi(argv[PARAMS_THIRD]);
    if ((errno != 0) || (deviceId < 0) || (deviceId > ID_MAX)) {
        return -1;
    }
    int vDeviceId = atoi(argv[PARAMS_FOURTH]);
    if ((errno != 0) || (vDeviceId < 0) || (vDeviceId > ID_MAX)) {
        return -1;
    }
    void *handle = NULL;
    if (!DeclareDcmiApiAndCheck(&handle)) {
        (void)fprintf(stderr, "Declare dcmi failed.\n");
        return -1;
    }
    int (*dcmi_init)(void) = NULL;
    int (*dcmi_set_destroy_vdevice)(int, int, int) = NULL;
    dcmi_init = dlsym(handle, DCMI_INIT);
    if (dcmi_init == NULL) {
        DcmiDlAbnormalExit(&handle, "DeclareDlApi failed");
        return -1;
    }
    dcmi_set_destroy_vdevice = dlsym(handle, DCMI_SET_DESTROY_VDEVICE);
    if (dcmi_set_destroy_vdevice == NULL) {
        DcmiDlAbnormalExit(&handle, "DeclareDlApi failed");
        return -1;
    }
    int ret = dcmi_init();
    if (ret != 0) {
        (void)fprintf(stderr, "dcmi_init failed, ret = %d\n", ret);
        DcmiDlclose(&handle);
        return -1;
    }
    ret = dcmi_set_destroy_vdevice(cardId, deviceId, vDeviceId);
    if (ret != 0) {
        (void)fprintf(stderr, "dcmi_set_destroy_vdevice failed, ret = %d\n", ret);
        DcmiDlclose(&handle);
        return -1;
    }
    DcmiDlclose(&handle);
    return ret;
}

static bool EntryCheck(const int argc, const char *argv[])
{
    if (argc != DESTROY_PARAMS_NUM) {
        (void)fprintf(stderr, "destroy params namber error.\n");
        return false;
    }
    for (int iLoop = 1; iLoop < argc; iLoop++) {
        for (size_t jLoop = 0; jLoop < strlen(argv[iLoop]); jLoop++) {
            if (isdigit(argv[iLoop][jLoop]) == 0) {
                return false;
            }
        }
    }
    return true;
}

int main(const int argc, const char *argv[])
{
    if (!EntryCheck(argc, argv)) {
        (void)fprintf(stderr, "destroy params value error.\n");
        return -1;
    }

    return DestroyEntrance(argv);
}
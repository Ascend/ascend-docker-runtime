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
#include "basic.h"
#include "logger.h"
#include "utils.h"

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
    Logger(exceptionInfo, LEVEL_ERROR, SCREEN_YES);
    return false;
}

static bool CheckFileOwner(const struct stat fileStat, const bool checkOwner)
{
    if (checkOwner) {
        if ((fileStat.st_uid != ROOT_UID) && (fileStat.st_uid != geteuid())) { // 操作文件owner非root/自己
            return ShowExceptionInfo("Please check the folder owner!");
        }
    }
    return true;
}

static bool CheckParentDir(char* buf, const size_t bufLen, struct stat fileStat, const bool checkOwner)
{
    if (buf == NULL) {
        return false;
    }
    for (int iLoop = 0; iLoop < PATH_MAX; iLoop++) {
        if (!CheckFileOwner(fileStat, checkOwner)) {
            return false;
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
    return CheckParentDir(buf, PATH_MAX, fileStat, checkOwner);
}

static bool IsAValidChar(const char c)
{
    if (isalnum(c) != 0) {
        return true;
    }
    // ._-/~为合法字符
    if ((c == '.') || (c == '_') ||
        (c == '-') || (c == '/') || (c == '~')) {
        return true;
    }
    return false;
}

static bool CheckFileName(const char* filePath, const size_t filePathLen)
{
    int iLoop;
    if ((filePathLen > PATH_MAX) || (filePathLen <= 0)) { // 长度越界
        return ShowExceptionInfo("filePathLen out of bounds!");
    }
    for (iLoop = 0; iLoop < filePathLen; iLoop++) {
        if (!IsAValidChar(filePath[iLoop])) { // 非法字符
            return ShowExceptionInfo("filePath has an illegal character!");
        }
    }
    return true;
}

static bool CheckAExternalFile(const char* filePath, const size_t filePathLen,
    const size_t maxFileSzieMb, const bool checkOwner)
{
    if (filePath == NULL) {
        return false;
    }
    if (!CheckFileName(filePath, filePathLen)) {
        return false;
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
        Logger("dlopen failed.\n", LEVEL_ERROR, SCREEN_YES);
        return false;
    }
    char pLinkMap[sizeof(struct link_map)] = {0};
    int ret = dlinfo(*handle, RTLD_DI_LINKMAP, &pLinkMap);
    if (ret == 0) {
        struct link_map* pLink = *(struct link_map**)pLinkMap;
        const size_t maxFileSzieMb = 10; // max 10 mb
        if (!CheckAExternalFile(pLink->l_name, strlen(pLink->l_name), maxFileSzieMb, true)) {
            Logger("check sofile failed.\n", LEVEL_ERROR, SCREEN_YES);
            return false;
        }
    } else {
        Logger("dlinfo sofile failed.\n", LEVEL_ERROR, SCREEN_YES);
        return false;
    }
    
    return true;
}

static void DcmiDlAbnormalExit(void **handle, const char* errorInfo)
{
    Logger(errorInfo, LEVEL_INFO, SCREEN_YES);
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

static bool CheckLimitId(const int IdValue)
{
    if (IdValue < 0 || IdValue > ID_MAX) {
        return false;
    }
    return true;
}

static bool GetAndCheckID(const char *argv[], int *cardId,
                          int *deviceId, int *vDeviceId)
{
    errno = 0;
    *cardId = atoi(argv[PARAMS_SECOND]);
    if ((errno != 0) || !CheckLimitId(*cardId)) {
        return false;
    }
    *deviceId = atoi(argv[PARAMS_THIRD]);
    if ((errno != 0) || !CheckLimitId(*deviceId)) {
        return false;
    }
    *vDeviceId = atoi(argv[PARAMS_FOURTH]);
    if ((errno != 0) || !CheckLimitId(*vDeviceId)) {
        return false;
    }
    return true;
}

static bool DcmiInitProcess(void *handle)
{
    if (handle == NULL) {
        return false;
    }
    int (*dcmi_init)(void) = NULL;
    dcmi_init = dlsym(handle, DCMI_INIT);
    if (dcmi_init == NULL) {
        DcmiDlAbnormalExit(&handle, "DeclareDlApi failed");
        return false;
    }
    int ret = dcmi_init();
    if (ret != 0) {
        Logger("dcmi_init faile.\n", LEVEL_ERROR, SCREEN_YES);
        DcmiDlclose(&handle);
        return false;
    }
    return true;
}

static bool DcmiDestroyProcess(void *handle, const int cardId,
                               const int deviceId, const int vDeviceId)
{
    if (handle == NULL) {
        return false;
    }
    int (*dcmi_set_destroy_vdevice)(int, int, int) = NULL;
    dcmi_set_destroy_vdevice = dlsym(handle, DCMI_SET_DESTROY_VDEVICE);
    if (dcmi_set_destroy_vdevice == NULL) {
        DcmiDlAbnormalExit(&handle, "DeclareDlApi failed");
        return false;
    }
    int ret = dcmi_set_destroy_vdevice(cardId, deviceId, vDeviceId);
    if (ret != 0) {
        Logger("dcmi_set_destroy_vdevice failed.\n", LEVEL_ERROR, SCREEN_YES);
        DcmiDlclose(&handle);
        return false;
    }
    return true;
}

static int DestroyEntrance(const char *argv[])
{
    if (argv == NULL) {
        return -1;
    }
    int cardId = 0;
    int deviceId = 0;
    int vDeviceId = 0;
    char *str = FormatLogMessage("start to destroy v-device %d start...\n", vDeviceId);
    Logger(str, LEVEL_INFO, SCREEN_YES);
    free(str);
    if (!GetAndCheckID(argv, &cardId, &deviceId, &vDeviceId)) {
        return -1;
    }

    void *handle = NULL;
    if (!DeclareDcmiApiAndCheck(&handle)) {
        Logger("Declare dcmi failed.\n", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    if (!DcmiInitProcess(handle)) {
        return -1;
    }
    if (!DcmiDestroyProcess(handle, cardId, deviceId, vDeviceId)) {
        return -1;
    }
    DcmiDlclose(&handle);
    char *strEnd = FormatLogMessage("destroy v-device %d successfully.\n", vDeviceId);
    Logger(strEnd, LEVEL_INFO, SCREEN_YES);
    free(strEnd);
    return 0;
}

static bool EntryCheck(const int argc, const char *argv[])
{
    if (argc != DESTROY_PARAMS_NUM) {
        Logger("destroy params namber error.\n", LEVEL_ERROR, SCREEN_YES);
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
        Logger("destroy params value error.\n", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return DestroyEntrance(argv);
}
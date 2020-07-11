/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器设备与驱动挂载模块
*/
#include "mount.h"

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include "securec.h"
#include "utils.h"
#include "logging.h"
#include "options.h"

static int GetDeviceMntSrcDst(const char *rootfs, const char *deviceName,
    struct PathInfo* pathInfo)
{
    int ret;
    errno_t err;
    char unresolvedDst[BUF_SIZE] = {0};
    char resolvedDst[PATH_MAX] = {0};

    ret = VerifyPathInfo(pathInfo);
    if (ret < 0) {
        return -1;
    }
    
    size_t srcBufSize = pathInfo->srcLen;
    size_t dstBufSize = pathInfo->dstLen;
    char *src = pathInfo->src;
    char *dst = pathInfo->dst;

    ret = snprintf_s(src, srcBufSize, srcBufSize, "/dev/%s", deviceName);
    if (ret < 0) {
        return -1;
    }

    ret = snprintf_s(unresolvedDst, BUF_SIZE, BUF_SIZE, "%s%s", rootfs, src);
    if (ret < 0) {
        return -1;
    }

    if (realpath(unresolvedDst, resolvedDst) == NULL && errno != ENOENT) {
        LogError("error: cannot canonicalize device dst: %s\n", dst);
        return -1;
    }

    err = strcpy_s(dst, dstBufSize, (const char *)resolvedDst);
    if (err != EOK) {
        LogError("error: failed to copy resolved device mnt path to dst: %s\n", resolvedDst);
        return -1;
    }

    return 0;
}

int MountDevice(const char *rootfs, const char *deviceName)
{
    int ret;
    char src[BUF_SIZE] = {0};
    char dst[BUF_SIZE] = {0};
    struct PathInfo pathInfo = {src, BUF_SIZE, dst, BUF_SIZE};

    ret = GetDeviceMntSrcDst(rootfs, deviceName, &pathInfo);
    if (ret < 0) {
        LogError("error: failed to get device mount src and(or) dst path, device name: %s\n", deviceName);
        return -1;
    }

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        LogError("error: failed to stat src: %s\n", src);
        return -1;
    }

    ret = CreateFile(dst, srcStat.st_mode);
    if (ret < 0) {
        LogError("error: failed to create mount dst file: %s\n", dst);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LogError("error: failed to mount dev\n");
        return -1;
    }

    return 0;
}

int DoDeviceMounting(const char *rootfs, const char *devicesList)
{
    static const char *sep = ",";
    char list[BUF_SIZE] = {0};
    char deviceName[BUF_SIZE] = {0};

    errno_t err = strncpy_s(list, BUF_SIZE, devicesList, strlen(devicesList));
    if (err != EOK) {
        return -1;
    }
    char *token = NULL;
    char *context = NULL;

    token = strtok_s(list, sep, &context);
    while (token != NULL) {
        int ret = snprintf_s(deviceName, BUF_SIZE, BUF_SIZE, "%s%s", DEVICE_NAME, token);
        if (ret < 0) {
            LogError("error: assemble device name failed, id: %s\n", token);
            return -1;
        }

        ret = MountDevice(rootfs, deviceName);
        if (ret < 0) {
            LogError("error: failed to mount device no. %s\n", token);
            return -1;
        }

        token = strtok_s(NULL, sep, &context);
    }

    return 0;
}

int MountDir(const char *rootfs, const char *src)
{
    int ret;
    char dst[BUF_SIZE] = {0};

    ret = snprintf_s(dst, BUF_SIZE, BUF_SIZE, "%s%s", rootfs, src);
    if (ret < 0) {
        return -1;
    }

    struct stat srcStat;
    ret = stat(src, &srcStat);
    if (ret < 0) {
        return -1;
    }

    /* directory */
    char parentDir[BUF_SIZE] = {0};
    GetParentPathStr(dst, parentDir, BUF_SIZE);
    if (CheckDirExists(parentDir) < 0) {
        mode_t parentMode = DEFAULT_DIR_MODE;
        ret = MakeParentDir(parentDir, parentMode);
        if (ret < 0) {
            LogError("error: failed to make dir: %s\n", parentDir);
            return -1;
        }
    }

    if (CheckDirExists(dst) < 0) {
        const mode_t curMode = srcStat.st_mode;
        ret = MkDir(dst, curMode);
        if (ret < 0) {
            LogError("error: failed to make dir: %s\n", dst);
            return -1;
        }
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LogError("error: failed to mount dir: %s to %s\n", src, dst);
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    /* device */
    int ret = MountDevice(rootfs, DAVINCI_MANAGER);
    if (ret < 0) {
        LogError("error: failed to mount device %s\n", DAVINCI_MANAGER);
        return -1;
    }

    ret = MountDevice(rootfs, DEVMM_SVM);
    if (ret < 0) {
        LogError("error: failed to mount device %s\n", DEVMM_SVM);
        return -1;
    }

    ret = MountDevice(rootfs, HISI_HDC);
    if (ret < 0) {
        LogError("error: failed to mount device %s\n", HISI_HDC);
        return -1;
    }

    return 0;
}

int DoDirectoryMounting(const char *rootfs)
{
    /* directory */
    int ret = MountDir(rootfs, ASCEND_DRIVER_PATH);
    if (ret < 0) {
        LogError("error: failed to do mount %s\n", ASCEND_DRIVER_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_ADDONS_PATH);
    if (ret < 0) {
        LogError("error: failed to do mount %s\n", ASCEND_ADDONS_PATH);
        return -1;
    }

    return 0;
}

int DoMounting(const struct CmdArgs *args)
{
    int ret;

    ret = DoDeviceMounting(args->rootfs, args->devices);
    if (ret < 0) {
        LogError("error: failed to do mounts\n");
        return -1;
    }

    ret = DoCtrlDeviceMounting(args->rootfs);
    if (ret < 0) {
        LogError("error: failed to do mount files\n");
        return -1;
    }

    if (IsOptionNoDrvSet()) {
        // 不挂载DRIVER
        return 0;
    }

    ret = DoDirectoryMounting(args->rootfs);
    if (ret < 0) {
        LogError("error: failed to do mount directory\n");
        return -1;
    }

    return 0;
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器设备与驱动挂载模块
*/
#include "mount.h"

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "securec.h"
#include "utils.h"
#include "logging.h"
#include "options.h"

int Mount(const char *src, const char *dst)
{
    static const unsigned long mountFlags = MS_BIND;
    static const unsigned long remountFlags = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID;
    int ret;

    ret = mount(src, dst, NULL, mountFlags, NULL);
    if (ret < 0) {
        LogError("error: failed to mount.");
        return -1;
    }

    ret = mount(NULL, dst, NULL, remountFlags, NULL);
    if (ret < 0) {
        LogError("error: failed to re-mount.");
        return -1;
    }

    return 0;
}

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

    ret = sprintf_s(src, srcBufSize, "/dev/%s", deviceName);
    if (ret < 0) {
        return -1;
    }

    ret = sprintf_s(unresolvedDst, BUF_SIZE, "%s%s", rootfs, src);
    if (ret < 0) {
        return -1;
    }

    if (realpath(unresolvedDst, resolvedDst) == NULL && errno != ENOENT) {
        LogError("error: cannot canonicalize device dst: %s.", dst);
        return -1;
    }

    err = strcpy_s(dst, dstBufSize, (const char *)resolvedDst);
    if (err != EOK) {
        LogError("error: failed to copy resolved device mnt path to dst: %s.", resolvedDst);
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
        LogError("error: failed to get device mount src and(or) dst path, device name: %s.", deviceName);
        return -1;
    }

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        LogError("error: failed to stat src: %s.", src);
        return -1;
    }

    ret = CreateFile(dst, srcStat.st_mode);
    if (ret < 0) {
        LogError("error: failed to create mount dst file: %s.", dst);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LogError("error: failed to mount dev.");
        return -1;
    }

    return 0;
}

int DoDeviceMounting(const char *rootfs, const unsigned int ids[], size_t idsNr)
{
    char deviceName[BUF_SIZE] = {0};

    for (size_t idx = 0; idx < idsNr; idx++) {
        int ret = sprintf_s(deviceName, BUF_SIZE, "%s%u", DEVICE_NAME, ids[idx]);
        if (ret < 0) {
            LogError("error: assemble device name failed, id: %u.", ids[idx]);
            return -1;
        }

        ret = MountDevice(rootfs, deviceName);
        if (ret < 0) {
            LogError("error: failed to mount device %s.", deviceName);
            return -1;
        }
    }

    return 0;
}

int MountDir(const char *rootfs, const char *src)
{
    int ret;
    char dst[BUF_SIZE] = {0};

    ret = sprintf_s(dst, BUF_SIZE, "%s%s", rootfs, src);
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
            LogError("error: failed to make dir: %s.", parentDir);
            return -1;
        }
    }

    if (CheckDirExists(dst) < 0) {
        const mode_t curMode = srcStat.st_mode;
        ret = MkDir(dst, curMode);
        if (ret < 0) {
            LogError("error: failed to make dir: %s.", dst);
            return -1;
        }
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LogError("error: failed to mount dir: %s to %s.", src, dst);
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    /* device */
    int ret = MountDevice(rootfs, DAVINCI_MANAGER);
    if (ret < 0) {
        LogError("error: failed to mount device %s.", DAVINCI_MANAGER);
        return -1;
    }

    ret = MountDevice(rootfs, DEVMM_SVM);
    if (ret < 0) {
        LogError("error: failed to mount device %s.", DEVMM_SVM);
        return -1;
    }

    ret = MountDevice(rootfs, HISI_HDC);
    if (ret < 0) {
        LogError("error: failed to mount device %s.", HISI_HDC);
        return -1;
    }

    return 0;
}

int DoDirectoryMounting(const char *rootfs)
{
    /* directory */
    int ret = MountDir(rootfs, ASCEND_DRIVER_PATH);
    if (ret < 0) {
        LogError("error: failed to do mount %s.", ASCEND_DRIVER_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_ADDONS_PATH);
    if (ret < 0) {
        LogError("error: failed to do mount %s.", ASCEND_ADDONS_PATH);
        return -1;
    }

    return 0;
}

int DoMounting(const struct ParsedConfig *config)
{
    int ret;

    ret = DoDeviceMounting(config->rootfs, config->devices, config->devicesNr);
    if (ret < 0) {
        LogError("error: failed to do mounts.");
        return -1;
    }

    ret = DoCtrlDeviceMounting(config->rootfs);
    if (ret < 0) {
        LogError("error: failed to do mount files.");
        return -1;
    }

    if (!IsOptionNoDrvSet()) {
        ret = DoDirectoryMounting(config->rootfs);
        if (ret < 0) {
            LogError("error: failed to do mount directory.");
            return -1;
        }
    }

    return 0;
}
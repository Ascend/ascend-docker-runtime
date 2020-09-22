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
        LOG_ERROR("error: failed to mount.");
        return -1;
    }

    ret = mount(NULL, dst, NULL, remountFlags, NULL);
    if (ret < 0) {
        LOG_ERROR("error: failed to re-mount.");
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
        LOG_ERROR("error: cannot canonicalize device dst: %s.", dst);
        return -1;
    }

    err = strcpy_s(dst, dstBufSize, (const char *)resolvedDst);
    if (err != EOK) {
        LOG_ERROR("error: failed to copy resolved device mnt path to dst: %s.", resolvedDst);
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
        LOG_ERROR("error: failed to get device mount src and(or) dst path, device name: %s.", deviceName);
        return -1;
    }

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        LOG_ERROR("error: failed to stat src: %s.", src);
        return -1;
    }

    ret = CreateFile(dst, srcStat.st_mode);
    if (ret < 0) {
        LOG_ERROR("error: failed to create mount dst file: %s.", dst);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount dev.");
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
            LOG_ERROR("error: assemble device name failed, id: %u.", ids[idx]);
            return -1;
        }

        ret = MountDevice(rootfs, deviceName);
        if (ret < 0) {
            LOG_ERROR("error: failed to mount device %s.", deviceName);
            return -1;
        }
    }

    return 0;
}

int MountFile(const char *rootfs, const char *filepath)
{
    int ret;
    char dst[BUF_SIZE] = {0};

    ret = sprintf_s(dst, BUF_SIZE, "%s%s", rootfs, filepath);
    if (ret < 0) {
        LOG_ERROR("error: failed to assemble file mounting path, file: %s.", filepath);
        return -1;
    }

    struct stat srcStat;
    ret = stat(filepath, &srcStat);
    if (ret < 0) {
        LOG_WARNING("warning: failed to find file %s on host, skipping", filepath);
        return 0;
    }

    if (!S_ISREG(srcStat.st_mode)) {
        LOG_ERROR("error: this should be a regular file to be mounted: %s.", filepath);
        return -1;
    }

    ret = CreateFile(dst, srcStat.st_mode);
    if (ret < 0) {
        LOG_ERROR("error: failed to create mount dst file: %s.", dst);
        return -1;
    }

    ret = Mount(filepath, dst);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount dev.");
        return -1;
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
        LOG_WARNING("warning: failed to find dir %s on host, skipping", src);
        return 0;
    }

    if (!S_ISDIR(srcStat.st_mode)) {
        LOG_ERROR("error: this should be a directory to be mounted: %s.", src);
        return -1;
    }

    ret = MakeDirWithParent(dst, DEFAULT_DIR_MODE);
    if (ret < 0) {
        LOG_ERROR("error: failed to make dir: %s.", dst);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount dir: %s to %s.", src, dst);
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    /* device */
    int ret = MountDevice(rootfs, DAVINCI_MANAGER);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount device %s.", DAVINCI_MANAGER);
        return -1;
    }

    ret = MountDevice(rootfs, DEVMM_SVM);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount device %s.", DEVMM_SVM);
        return -1;
    }

    ret = MountDevice(rootfs, HISI_HDC);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount device %s.", HISI_HDC);
        return -1;
    }

    return 0;
}

int DoDirectoryMounting(const char *rootfs)
{
    /* directory */
    int ret = MountDir(rootfs, ASCEND_DRIVER_LIB64_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_DRIVER_LIB64_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_DRIVER_TOOLS_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_DRIVER_TOOLS_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_DRIVER_INC_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_DRIVER_INC_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_ADDONS_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_ADDONS_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_DCMI_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_DCMI_PATH);
        return -1;
    }

    return 0;
}

int DoFileMounting(const char *rootfs)
{
    int ret = MountFile(rootfs, ASCEND_NPU_SMI_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_NPU_SMI_PATH);
        return -1;
    }

    ret = MountFile(rootfs, ASCEND_NPU_SMI_PATH_OLD);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_NPU_SMI_PATH_OLD);
        return -1;
    }

    ret = MountFile(rootfs, ASCEND_SLOG_CONF_PATH);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount %s.", ASCEND_SLOG_CONF_PATH);
        return -1;
    }

    return 0;
}

int DoMounting(const struct ParsedConfig *config)
{
    int ret;

    ret = DoDeviceMounting(config->rootfs, config->devices, config->devicesNr);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount devices.");
        return -1;
    }

    ret = DoCtrlDeviceMounting(config->rootfs);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount ctrl devices.");
        return -1;
    }

    if (IsOptionNoDrvSet()) {
        return 0;
    }

    ret = DoFileMounting(config->rootfs);
    if (ret < 0) {
        LOG_ERROR("error: failed to mount files.");
        return -1;
    }

    ret = DoDirectoryMounting(config->rootfs);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mount directories.");
        return -1;
    }

    return 0;
}
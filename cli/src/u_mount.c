/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器设备与驱动挂载模块
*/
#include "u_mount.h"

#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "securec.h"

#include "basic.h"
#include "utils.h"
#include "options.h"
#include "logger.h"

int Mount(const char *src, const char *dst)
{
    if (src == NULL || dst == NULL) {
        Logger("src pointer or dst pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    static const unsigned long mountFlags = MS_BIND;
    static const unsigned long remountFlags = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID;
    int ret;
    struct stat fileStat;
    if ((stat(src, &fileStat) == 0) &&
        ((S_ISREG(fileStat.st_mode) != 0) || (S_ISDIR(fileStat.st_mode) != 0))) { // 只校验文件和目录
            const size_t maxFileSzieMb = 10 * 1024; // max 10 G
            if (!CheckExternalFile(src, strlen(src), maxFileSzieMb, false)) {
                char* str = FormatLogMessage("failed to mount src: %s.", src);
                Logger(str, LEVEL_ERROR, SCREEN_YES);
                return -1;
            }
    }
    ret = mount(src, dst, NULL, mountFlags, NULL);
    if (ret < 0) {
        Logger("failed to mount src.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = mount(NULL, dst, NULL, remountFlags, NULL);
    if (ret < 0) {
        Logger("failed to re-mount. dst.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}

static int GetDeviceMntSrcDst(const char *rootfs, const char *srcDeviceName,
                              const char *dstDeviceName, struct PathInfo* pathInfo)
{
    if (rootfs == NULL || srcDeviceName == NULL || pathInfo == NULL) {
        Logger("rootfs, srcDeviceName, pathInfo pointer are null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

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

    ret = sprintf_s(src, srcBufSize, "/dev/%s", srcDeviceName);
    if (ret < 0) {
        return -1;
    }
    ret = sprintf_s(unresolvedDst, BUF_SIZE, "%s%s", rootfs, src);
    if (ret < 0) {
        return -1;
    }
    if (realpath(unresolvedDst, resolvedDst) == NULL && errno != ENOENT) {
        Logger("cannot canonicalize device dst.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    if (dstDeviceName != NULL) {
        ret = sprintf_s(dst, dstBufSize, "%s/dev/%s", rootfs, dstDeviceName);
        if (ret < 0) {
            return -1;
        }
    } else {
        err = strcpy_s(dst, dstBufSize, resolvedDst);
        if (err != EOK) {
            Logger("failed to copy resolved device mnt path to dst.", LEVEL_ERROR, SCREEN_YES);
            return -1;
        }
    }

    return 0;
}

int MountDevice(const char *rootfs, const char *srcDeviceName, const char *dstDeviceName)
{
    int ret;
    char *str = NULL;
    char src[BUF_SIZE] = {0};
    char dst[BUF_SIZE] = {0};
    struct PathInfo pathInfo = {src, BUF_SIZE, dst, BUF_SIZE};
    ret = GetDeviceMntSrcDst(rootfs, srcDeviceName, dstDeviceName, &pathInfo);
    if (ret < 0) {
        Logger("failed to get mount src and dst path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        Logger("failed to stat src.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    errno = 0;
    struct stat dstStat;
    ret = stat((const char *)dst, &dstStat);
    if (ret == 0 && S_ISCHR(dstStat.st_mode)) {
        return 0; // 特权容器自动挂载HOST所有设备，故此处跳过
    } else if (ret == 0) {
        Logger("dst already exists but not a char device as expected.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    } else if (ret < 0 && errno != ENOENT) {
        Logger("failed to check dst stat", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    ret = MakeMountPoints(dst, srcStat.st_mode);
    if (ret < 0) {
        Logger("failed to create mount dst file.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        Logger("failed to mount dev.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    free(str);
    return 0;
}

int DoDeviceMounting(const char *rootfs, const char *device_name, const unsigned int ids[], size_t idsNr)
{
    if (rootfs == NULL || device_name == NULL) {
        Logger("rootfs, device_name pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    char srcDeviceName[BUF_SIZE] = {0};
    char dstDeviceName[BUF_SIZE] = {0};

    for (size_t idx = 0; idx < idsNr; idx++) {
        int srcRet = sprintf_s(srcDeviceName, BUF_SIZE, "%s%u", device_name, ids[idx]);
        int dstRet = sprintf_s(dstDeviceName, BUF_SIZE, "%s%u", DEVICE_NAME, ids[idx]);
        if (srcRet < 0 || dstRet < 0) {
            char* str = FormatLogMessage("assemble device name failed, id: %u.", ids[idx]);
            Logger(str, LEVEL_ERROR, SCREEN_YES);
            free(str);
            return -1;
        }
        int ret = MountDevice(rootfs, srcDeviceName, dstDeviceName);
        if (ret < 0) {
            Logger("failed to mount device.", LEVEL_ERROR, SCREEN_YES);
            return -1;
        }
    }

    return 0;
}

int MountFile(const char *rootfs, const char *filepath)
{
    if (rootfs == NULL || filepath == NULL) {
        Logger("rootfs, filepath pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int ret;
    char dst[BUF_SIZE] = {0};

    ret = sprintf_s(dst, BUF_SIZE, "%s%s", rootfs, filepath);
    if (ret < 0) {
        Logger("failed to assemble file mounting path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    struct stat srcStat;
    ret = stat(filepath, &srcStat);
    if (ret < 0) {
        return 0;
    }

    ret = MakeMountPoints(dst, srcStat.st_mode);
    if (ret < 0) {
        Logger("failed to create mount dst file.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = Mount(filepath, dst);
    if (ret < 0) {
        Logger("failed to mount dev.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}

int MountDir(const char *rootfs, const char *src)
{
    if (rootfs == NULL || src == NULL) {
        Logger("rootfs, src pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int ret;
    char dst[BUF_SIZE] = {0};

    ret = sprintf_s(dst, BUF_SIZE, "%s%s", rootfs, src);
    if (ret < 0) {
        return -1;
    }

    struct stat srcStat;
    ret = stat(src, &srcStat);
    if (ret < 0) {
        return 0;
    }

    ret = MakeDirWithParent(dst, DEFAULT_DIR_MODE);
    if (ret < 0) {
        Logger("failed to make dir.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        Logger("failed to mount dir", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    if (rootfs == NULL) {
        Logger("rootfs pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    /* device */
    int ret = MountDevice(rootfs, DAVINCI_MANAGER, NULL);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to mount device %s.", DAVINCI_MANAGER); // error1
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    char devmmPath[PATH_MAX] = {0};
    char hisiPath[PATH_MAX] = {0};
    if ((sprintf_s(devmmPath, PATH_MAX, "/dev/%s", DEVMM_SVM) < 0) ||
        (sprintf_s(hisiPath, PATH_MAX, "/dev/%s", HISI_HDC) < 0)) {
        Logger("failed to assemble dev path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    struct stat devStat; // 200 soc 不需要挂载此两个设备
    if ((stat(devmmPath, &devStat) != 0) && (stat(hisiPath, &devStat) != 0)) {
        Logger("200 Soc.", LEVEL_ERROR, SCREEN_YES);
        return 0;
    }

    ret = MountDevice(rootfs, DEVMM_SVM, NULL);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to mount device %s.", DEVMM_SVM);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    ret = MountDevice(rootfs, HISI_HDC, NULL);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to mount device %s.", HISI_HDC);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    return 0;
}

int DoDirectoryMounting(const char *rootfs, const struct MountList *list)
{
    if (rootfs == NULL || list == NULL) {
        Logger("rootfs, list pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int ret;
    for (unsigned int i = 0; i < list->count; i++) {
        ret = MountDir(rootfs, (const char *)&list->list[i][0]);
        if (ret < 0) {
            Logger("failed to do directory mounting", LEVEL_ERROR, SCREEN_YES);
            return -1;
        }
    }

    return 0;
}

int DoFileMounting(const char *rootfs, const struct MountList *list)
{
    if (rootfs == NULL || list == NULL) {
        Logger("rootfs, list pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int ret;
    for (unsigned int i = 0; i < list->count; i++) {
        ret = MountFile(rootfs, (const char *)&list->list[i][0]);
        if (ret < 0) {
            char* str = FormatLogMessage("failed to do file mounting for %s.", (const char *)&list->list[i][0]);
            Logger(str, LEVEL_ERROR, SCREEN_YES);
            free(str);
            return -1;
        }
    }

    return 0;
}

int DoMounting(const struct ParsedConfig *config)
{
    if (config == NULL) {
        Logger("config pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    int ret;
    ret = DoDeviceMounting(config->rootfs,
                           (IsVirtual() ? VDEVICE_NAME : DEVICE_NAME),
                           config->devices, config->devicesNr);
    if (ret < 0) {
        Logger("failed to mount devices.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = DoCtrlDeviceMounting(config->rootfs);
    if (ret < 0) {
        Logger("failed to mount ctrl devices.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    if (IsOptionNoDrvSet()) {
        return 0;
    }

    ret = DoFileMounting(config->rootfs, config->files);
    if (ret < 0) {
        Logger("failed to mount files.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    ret = DoDirectoryMounting(config->rootfs, config->dirs);
    if (ret < 0) {
        Logger("failed to do mount directories.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}
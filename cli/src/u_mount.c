/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

static bool checkSrcFile(const char *src)
{
    struct stat fileStat;
    if (lstat(src, &fileStat) != 0) {
        return -1; // 待挂载源文件不存在
    }
    if ((S_ISREG(fileStat.st_mode) != 0) || (S_ISDIR(fileStat.st_mode) != 0)) { // 只校验文件和目录
            const size_t maxFileSzieMb = 10 * 1024; // max 10 G
            if (!CheckExternalFile(src, strlen(src), maxFileSzieMb, false)) {
                char* str = FormatLogMessage("failed to mount src: %s.", src);
                Logger(str, LEVEL_ERROR, SCREEN_YES);
                return false;
            }
    }
    if (S_ISDIR(fileStat.st_mode) != 0) { // 目录则增加递归校验子集
        if (!GetFileSubsetAndCheck(src, strlen(src))) {
            char* str = FormatLogMessage("Check file subset failed: %s.", src);
            Logger(str, LEVEL_ERROR, SCREEN_YES);
            free(str);
            return false;
        }
    }
    return true;
}

int Mount(const char *src, const char *dst)
{
    if (src == NULL || dst == NULL) {
        Logger("src pointer or dst pointer is null!", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    static const unsigned long mountFlags = MS_BIND;
    static const unsigned long remountFlags = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID;
    if (!checkSrcFile(src)) {
        return -1;
    }
    int ret = mount(src, dst, NULL, mountFlags, NULL);
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
            Logger("failed to do file mounting for.", LEVEL_ERROR, SCREEN_YES);
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
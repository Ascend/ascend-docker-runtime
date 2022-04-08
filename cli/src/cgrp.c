/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器CGroup配置模块
*/
#include "cgrp.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>

#include "securec.h"

#include "u_mount.h"
#include "utils.h"
#include "options.h"
#include "logger.h"

bool TakeNthWord(char **pLine, unsigned int n, char **word)
{
    if (pLine == NULL || word == NULL) {
        (void)fprintf(stderr, "pLine, word pointer is null!\n");
        return false;
    }

    char *w = NULL;
    for (unsigned int i = 0; i < n; i++) {
        w = strsep(pLine, " ");
        if (w == NULL || *w == '\0') {
            return false;
        }
    }

    *word = w;
    return true;
}

bool CheckRootDir(char **pLine)
{
    if (pLine == NULL) {
        (void)fprintf(stderr, "pLine pointer is null!\n");
        return false;
    }

    char *rootDir = NULL;
    if (!TakeNthWord(pLine, ROOT_GAP, &rootDir)) {
        return false;
    }

    return strlen(rootDir) < BUF_SIZE && !StrHasPrefix(rootDir, "/..");
}

bool CheckFsType(char **pLine)
{
    if (pLine == NULL) {
        (void)fprintf(stderr, "pLine pointer is null!\n");
        return false;
    }

    char* fsType = NULL;
    if (!TakeNthWord(pLine, FSTYPE_GAP, &fsType)) {
        return false;
    }

    return IsStrEqual(fsType, "cgroup");
}

bool CheckSubStr(char **pLine, const char *subsys)
{
    if (pLine == NULL || subsys == NULL) {
        (void)fprintf(stderr, "pLine, subsys pointer is null!\n");
        return false;
    }

    char* substr = NULL;
    if (!TakeNthWord(pLine, MOUNT_SUBSTR_GAP, &substr)) {
        return false;
    }

    return strstr(substr, subsys) != NULL;
}
typedef char *(*ParseFileLine)(char *, const char *);
static bool GetFileInfo(const char* resolvedPath, char* buffer, const int bufferSize, const ParseFileLine fn)
{
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    char *result = NULL;
    fp = fopen(resolvedPath, "r");
    if (fp == NULL) {
        Logger("cannot open file.", LEVEL_ERROR, SCREEN_YES);
        return false;
    }

    while (getline(&line, &len, fp) != -1) {
        result = fn(line, "devices");
        if (result != NULL && strlen(result) < bufferSize) {
            break;
        }
    }

    errno_t ret = strcpy_s(buffer, bufferSize, result);
    free(line);
    fclose(fp);
    if (ret != EOK) {
        return false;
    }
    return true;
}

int ParseFileByLine(char* buffer, int bufferSize, const ParseFileLine fn, const char* filepath)
{
    if (buffer == NULL || filepath == NULL) {
        (void)fprintf(stderr, "buffer, filepath pointer is null!\n");
        return -1;
    }
    
    char resolvedPath[PATH_MAX] = {0x0};

    if (realpath(filepath, resolvedPath) == NULL && errno != ENOENT) {
        Logger("Cannot canonicalize path.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    const size_t maxFileSzieMb = 1024; // max 1G
    if (!CheckExternalFile(resolvedPath, strlen(resolvedPath), maxFileSzieMb, true)) {
        Logger("Check file legality failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    if (!GetFileInfo(resolvedPath, buffer, bufferSize, fn)) {
        return -1;
    }

    return 0;
}

char *GetCgroupMount(char *line, const char *subsys)
{
    if (line == NULL || subsys == NULL) {
        (void)fprintf(stderr, "line, subsys pointer is null!\n");
        return NULL;
    }
    
    if (!CheckRootDir(&line)) {
        return NULL;
    }

    char *mountPoint = NULL;
    mountPoint = strsep(&line, " ");
    line = strchr(line, '-');
    if (mountPoint == NULL || *mountPoint == '\0') {
        return NULL;
    }

    if (!CheckFsType(&line)) {
        return NULL;
    }

    if (!CheckSubStr(&line, subsys)) {
        return NULL;
    }

    return mountPoint;
}

char *GetCgroupRoot(char *line, const char *subSystem)
{
    if (line == NULL || subSystem == NULL) {
        (void)fprintf(stderr, "line, subSystem pointer is null!\n");
        return NULL;
    }
    
    char *token = NULL;
    int i;
    for (i = 0; i < ROOT_SUBSTR_GAP; ++i) {
        token = strsep(&line, ":");
        if (token == NULL || *token == '\0') {
            return NULL;
        }
    }

    char *rootDir = strsep(&line, ":");
    if (rootDir == NULL || *rootDir == '\0') {
        return NULL;
    }

    if (strlen(rootDir) >= BUF_SIZE || StrHasPrefix(rootDir, "/..")) {
        return NULL;
    }
    if (strstr(token, subSystem) == NULL) {
        return NULL;
    }
    return rootDir;
}

int SetupDeviceCgroup(FILE *cgroupAllow, const char *devName)
{
    if (cgroupAllow == NULL || devName == NULL) {
        (void)fprintf(stderr, "cgroupAllow, devName pointer is null!\n");
        return -1;
    }

    int ret;
    struct stat devStat;
    char devPath[BUF_SIZE];

    ret = sprintf_s(devPath, BUF_SIZE, "/dev/%s", devName);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to assemble dev path for %s.", devName);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    ret = stat((const char *)devPath, &devStat);
    if (ret < 0) {
        Logger("Failed to get stat of devpath.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    bool isFailed = fprintf(cgroupAllow, "c %u:%u rw", major(devStat.st_rdev), minor(devStat.st_rdev)) < 0 ||
                    fflush(cgroupAllow) == EOF || ferror(cgroupAllow) < 0;
    if (isFailed) {
        Logger("write devices failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}

int SetupDriverCgroup(FILE *cgroupAllow)
{
    if (cgroupAllow == NULL) {
        (void)fprintf(stderr, "cgroupAllow pointer is null!\n");
        return -1;
    }

    int ret;
    ret = SetupDeviceCgroup(cgroupAllow, DAVINCI_MANAGER);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to setup cgroup for %s.", DAVINCI_MANAGER);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }
 
    bool is200Rc = false;
    if (!DoMounting200RC(&is200Rc)) {
        return -1;
    }
    if (is200Rc) {
        return 0;
    }

    ret = SetupDeviceCgroup(cgroupAllow, DEVMM_SVM);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to setup cgroup for %s.", DEVMM_SVM);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, HISI_HDC);
    if (ret < 0) {
        char* str = FormatLogMessage("failed to setup cgroup for %s.", HISI_HDC);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    return 0;
}

int GetCgroupPath(int pid, char *effPath, size_t maxSize)
{
    if (effPath == NULL) {
        (void)fprintf(stderr, "effPath pointer is null!\n");
        return -1;
    }

    int ret;
    char mountPath[BUF_SIZE] = {0x0};
    char mount[BUF_SIZE] = {0x0};

    ret = sprintf_s(mountPath, BUF_SIZE, "/proc/%d/mountinfo", (int)getppid());
    if (ret < 0) {
        char* str = FormatLogMessage("assemble mount info path failed: ppid(%d).", getppid());
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    ret = ParseFileByLine(mount, BUF_SIZE, GetCgroupMount, mountPath);
    if (ret < 0) {
        Logger("cat file content failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    char cgroup[BUF_SIZE] = {0x0};
    char cgroupPath[BUF_SIZE] = {0x0};
    ret = sprintf_s(cgroupPath, BUF_SIZE, "/proc/%d/cgroup", pid);
    if (ret < 0) {
        char* str = FormatLogMessage("assemble cgroup path failed: pid(%d).", pid);
        Logger(str, LEVEL_ERROR, SCREEN_YES);
        free(str);
        return -1;
    }

    ret = ParseFileByLine(cgroup, BUF_SIZE, GetCgroupRoot, cgroupPath);
    if (ret < 0) {
        Logger("cat file content failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    // cut last '\n' off
    cgroup[strcspn(cgroup, "\n")] = '\0';

    ret = sprintf_s(effPath, maxSize, "%s%s%s", mount, cgroup, ALLOW_PATH);
    if (ret < 0) {
        Logger("assemble cgroup device path failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }

    return 0;
}

static bool SetupCgroupProcess(const char* resolvedCgroupPath, const struct ParsedConfig *config)
{
    char *str = NULL;
    FILE *cgroupAllow = NULL;
    char deviceName[BUF_SIZE] = {0};
    cgroupAllow = fopen(resolvedCgroupPath, "a");
    if (cgroupAllow == NULL) {
        Logger("failed to open cgroup file.", LEVEL_ERROR, SCREEN_YES);
        return false;
    }

    if (SetupDriverCgroup(cgroupAllow) < 0) {
        fclose(cgroupAllow);
        Logger("failed to setup driver cgroup.", LEVEL_ERROR, SCREEN_YES);
        return false;
    }
    for (size_t idx = 0; idx < config->devicesNr; idx++) {
        if (sprintf_s(deviceName, BUF_SIZE, "%s%u",
            (IsVirtual() ? VDEVICE_NAME : DEVICE_NAME),
            config->devices[idx]) < 0) {
            fclose(cgroupAllow);
            str = FormatLogMessage("failed to assemble device path for no.%u.", config->devices[idx]);
            Logger(str, LEVEL_ERROR, SCREEN_YES);
            free(str);
            return false;
        }

        if (SetupDeviceCgroup(cgroupAllow, (const char *)deviceName) < 0) {
            fclose(cgroupAllow);
            Logger("failed to setup cgroup.", LEVEL_ERROR, SCREEN_YES);
            return false;
        }
    }
    free(str);
    fclose(cgroupAllow);
    return true;
}

int SetupCgroup(const struct ParsedConfig *config)
{
    if (config == NULL) {
        (void)fprintf(stderr, "config pointer is null!\n");
        return -1;
    }

    char resolvedCgroupPath[PATH_MAX] = {0};
    if (realpath(config->cgroupPath, resolvedCgroupPath) == NULL && errno != ENOENT) {
        Logger("cannot canonicalize cgroup.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    const size_t maxFileSzieMb = 1024; // max 1G
    if (!CheckExternalFile(resolvedCgroupPath, strlen(resolvedCgroupPath), maxFileSzieMb, true)) {
        Logger("Check file legality failed.", LEVEL_ERROR, SCREEN_YES);
        return -1;
    }
    if (!SetupCgroupProcess(resolvedCgroupPath, config)) {
        return -1;
    }

    return 0;
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具容器CGroup配置模块
*/
#include "cgrp.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "securec.h"
#include "utils.h"
#include "logging.h"

bool TakeNthWord(char **pLine, unsigned int n, char **word)
{
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
    char *rootDir = NULL;
    if (!TakeNthWord(pLine, ROOT_GAP, &rootDir)) {
        return false;
    }

    return strlen(rootDir) < BUF_SIZE && !StrHasPrefix(rootDir, "/..");
}

bool CheckFsType(char **pLine)
{
    char* fsType = NULL;
    if (!TakeNthWord(pLine, FSTYPE_GAP, &fsType)) {
        return false;
    }

    return IsStrEqual(fsType, "cgroup");
}

bool CheckSubStr(char **pLine, const char *subsys)
{
    char* substr = NULL;
    if (!TakeNthWord(pLine, MOUNT_SUBSTR_GAP, &substr)) {
        return false;
    }

    return strstr(substr, subsys) != NULL;
}

typedef char *(*ParseFileLine)(char *, const char *);
int ParseFileByLine(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath)
{
    FILE *fp = NULL;
    char *result = NULL;
    char *line = NULL;
    size_t len = 0;
    char resolvedPath[PATH_MAX] = {0x0};

    if (realpath(filepath, resolvedPath) == NULL && errno != ENOENT) {
        LogError("error: cannot canonicalize path %s.", filepath);
        return -1;
    }

    fp = fopen(resolvedPath, "r");
    if (fp == NULL) {
        LogError("cannot open file.");
        return -1;
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
        return -1;
    }

    return 0;
}

char *GetCgroupMount(char *line, const char *subsys)
{
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
    int ret;
    struct stat devStat;
    char devPath[BUF_SIZE];

    ret = sprintf_s(devPath, BUF_SIZE, "/dev/%s", devName);
    if (ret < 0) {
        LogError("error: failed to assemble dev path for %s.", devName);
        return -1;
    }

    ret = stat((const char *)devPath, &devStat);
    if (ret < 0) {
        LogError("error: failed to get stat of %s.", devPath);
        return -1;
    }

    bool isFailed = fprintf(cgroupAllow, "c %u:%u rw", major(devStat.st_rdev), minor(devStat.st_rdev)) < 0 ||
                    fflush(cgroupAllow) == EOF || ferror(cgroupAllow) < 0;
    if (isFailed) {
        LogError("error: write devices failed.");
        return -1;
    }

    return 0; 
}

int SetupDriverCgroup(FILE *cgroupAllow)
{
    int ret;

    ret = SetupDeviceCgroup(cgroupAllow, DAVINCI_MANAGER);
    if (ret < 0) {
        LogError("error: failed to setup cgroup for %s.", DAVINCI_MANAGER);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, DEVMM_SVM);
    if (ret < 0) {
        LogError("error: failed to setup cgroup for %s.", DEVMM_SVM);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, HISI_HDC);
    if (ret < 0) {
        LogError("error: failed to setup cgroup for %s.", HISI_HDC);
        return -1;
    }

    return 0;
}

int GetCgroupPath(int pid, char *effPath, size_t maxSize)
{
    int ret;
    char mountPath[BUF_SIZE] = {0x0};
    char mount[BUF_SIZE] = {0x0};

    ret = sprintf_s(mountPath, BUF_SIZE, "/proc/%d/mountinfo", (int)getppid());
    if (ret < 0) {
        LogError("error: assemble mount info path failed: ppid(%d).", getppid());
        return -1;
    }

    ret = ParseFileByLine(mount, BUF_SIZE, GetCgroupMount, mountPath);
    if (ret < 0) {
        LogError("error: cat file content failed.");
        return -1;
    }

    char cgroup[BUF_SIZE] = {0x0};
    char cgroupPath[BUF_SIZE] = {0x0};
    ret = sprintf_s(cgroupPath, BUF_SIZE, "/proc/%d/cgroup", pid);
    if (ret < 0) {
        LogError("error: assemble cgroup path failed: pid(%d).", pid);
        return -1;
    }

    ret = ParseFileByLine(cgroup, BUF_SIZE, GetCgroupRoot, cgroupPath);
    if (ret < 0) {
        LogError("error: cat file content failed.");
        return -1;
    }

    // cut last '\n' off
    cgroup[strcspn(cgroup, "\n")] = '\0';

    ret = sprintf_s(effPath, maxSize, "%s%s%s", mount, cgroup, ALLOW_PATH);
    if (ret < 0) {
        LogError("error: assemble cgroup device path failed.");
        return -1;
    }

    return 0;
}

int SetupCgroup(const struct ParsedConfig *config)
{
    int ret;
    char deviceName[BUF_SIZE] = {0};
    char resolvedCgroupPath[PATH_MAX] = {0};
    FILE *cgroupAllow = NULL;

    if (realpath(config->cgroupPath, resolvedCgroupPath) == NULL && errno != ENOENT) {
        LogError("error: cannot canonicalize cgroup path: %s.", config->cgroupPath);
        return -1;
    }

    cgroupAllow = fopen((const char *)resolvedCgroupPath, "a");
    if (cgroupAllow == NULL) {
        LogError("error: failed to open cgroup file: %s.", resolvedCgroupPath);
        return -1;
    }

    ret = SetupDriverCgroup(cgroupAllow);
    if (ret < 0) {
        fclose(cgroupAllow);
        LogError("error: failed to setup driver cgroup.");
        return -1;
    }

    for (size_t idx = 0; idx < config->devicesNr; idx++) {
        ret = sprintf_s(deviceName, BUF_SIZE, "%s%u", DEVICE_NAME, config->devices[idx]);
        if (ret < 0) {
            fclose(cgroupAllow);
            LogError("error: failed to assemble device path for no.%u.", config->devices[idx]);
            return -1;
        }

        ret = SetupDeviceCgroup(cgroupAllow, (const char *)deviceName);
        if (ret < 0) {
            fclose(cgroupAllow);
            LogError("error: failed to setup cgroup for %s.", deviceName);
            return -1;
        }
    }

    fclose(cgroupAllow);
    return 0;
}
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

    ret = snprintf_s(devPath, BUF_SIZE, BUF_SIZE, "/dev/%s", devName);
    if (ret < 0) {
        logError("error: failed to assemble dev path for %s\n", devName);
        return -1;
    }

    ret = stat((const char *)devPath, &devStat);
    if (ret < 0) {
        logError("error: failed to get stat of %s\n", devPath);
        return -1;
    }

    bool isFailed = fprintf(cgroupAllow, "c %u:%u rw", major(devStat.st_rdev), minor(devStat.st_rdev)) < 0 ||
                    fflush(cgroupAllow) == EOF || ferror(cgroupAllow) < 0;
    if (isFailed) {
        logError("error: write devices failed\n");
        return -1;
    }

    return 0; 
}

int SetupDriverCgroup(FILE *cgroupAllow)
{
    int ret;

    ret = SetupDeviceCgroup(cgroupAllow, DAVINCI_MANAGER);
    if (ret < 0) {
        logError("error: failed to setup cgroup for %s\n", DAVINCI_MANAGER);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, DEVMM_SVM);
    if (ret < 0) {
        logError("error: failed to setup cgroup for %s\n", DEVMM_SVM);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, HISI_HDC);
    if (ret < 0) {
        logError("error: failed to setup cgroup for %s\n", HISI_HDC);
        return -1;
    }

    return 0;
}

int GetCgroupPath(const struct CmdArgs *args, char *effPath, const size_t maxSize)
{
    int ret;
    char mountPath[BUF_SIZE] = {0x0};
    char mount[BUF_SIZE] = {0x0};

    ret = snprintf_s(mountPath, BUF_SIZE, BUF_SIZE, "/proc/%d/mountinfo", (int)getppid());
    if (ret < 0) {
        logError("error: assemble mount info path failed: ppid(%d)\n", getppid());
        return -1;
    }

    ret = CatFileContent(mount, BUF_SIZE, GetCgroupMount, mountPath);
    if (ret < 0) {
        logError("error: cat file content failed\n");
        return -1;
    }

    char cgroup[BUF_SIZE] = {0x0};
    char cgroupPath[BUF_SIZE] = {0x0};
    ret = snprintf_s(cgroupPath, BUF_SIZE, BUF_SIZE, "/proc/%d/cgroup", args->pid);
    if (ret < 0) {
        logError("error: assemble cgroup path failed: pid(%d)\n", args->pid);
        return -1;
    }

    ret = CatFileContent(cgroup, BUF_SIZE, GetCgroupRoot, cgroupPath);
    if (ret < 0) {
        logError("error: cat file content failed\n");
        return -1;
    }

    // cut last '\n' off
    cgroup[strcspn(cgroup, "\n")] = '\0';

    ret = snprintf_s(effPath, BUF_SIZE, maxSize, "%s%s%s", mount, cgroup, ALLOW_PATH);
    if (ret < 0) {
        logError("error: assemble cgroup device path failed: \n");
        return -1;
    }

    return 0;
}

int SetupCgroup(struct CmdArgs *args, const char *cgroupPath)
{
    int ret;
    char deviceName[BUF_SIZE] = {0};
    char resolvedCgroupPath[PATH_MAX] = {0};
    FILE *cgroupAllow = NULL;

    if (realpath(cgroupPath, resolvedCgroupPath) == NULL && errno != ENOENT) {
        logError("error: cannot canonicalize cgroup path: %s\n", cgroupPath);
        return -1;
    }

    static const char *sep = ",";
    char list[BUF_SIZE] = {0};
    errno_t err = strncpy_s(list, BUF_SIZE, args->devices, strlen(args->devices));
    if (err != EOK) {
        return -1;
    }
    char *token = NULL;

    cgroupAllow = fopen((const char *)resolvedCgroupPath, "a");
    if (cgroupAllow == NULL) {
        logError("error: failed to open cgroup file: %s\n", resolvedCgroupPath);
        return -1;
    }

    ret = SetupDriverCgroup(cgroupAllow);
    if (ret < 0) {
        fclose(cgroupAllow);
        logError("error: failed to setup driver cgroup\n");
        return -1;
    }
    
    token = strtok(list, sep);
    while (token != NULL) {
        ret = snprintf_s(deviceName, BUF_SIZE, BUF_SIZE, "%s%s", DEVICE_NAME, token);
        if (ret < 0) {
            fclose(cgroupAllow);
            logError("error: failed to assemble device path for no.%s\n", token);
            return -1;
        }

        ret = SetupDeviceCgroup(cgroupAllow, (const char *)deviceName);
        if (ret < 0) {
            fclose(cgroupAllow);
            logError("error: failed to setup cgroup %s\n", token);
            return -1;
        }

        token = strtok(NULL, sep);
    }

    fclose(cgroupAllow);
    return 0;
}
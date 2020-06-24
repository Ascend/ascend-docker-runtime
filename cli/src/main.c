/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具，配置容器挂载Ascend NPU设备
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <sys/fsuid.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <sched.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include "securec.h"

#define DEVICE_NAME "davinci"
#define DAVINCI_MANAGER             "davinci_manager"
#define DEVMM_SVM                   "devmm_svm"
#define HISI_HDC                    "hisi_hdc"
#define ASCEND_DRIVER_PATH          "/usr/local/Ascend/driver"
#define ASCEND_ADDONS_PATH          "/usr/local/Ascend/add-ons"
#define DEFAULT_DIR_MODE 0755
#define BUF_SIZE 1024
#define ALLOW_PATH "/devices.allow"
#define ROOT_GAP 4
#define FSTYPE_GAP 2
#define MOUNT_SUBSTR_GAP 2
#define ROOT_SUBSTR_GAP 2

static struct option g_cmdOpts[] = {
    {"devices", required_argument, 0, 'd'},
    {"pid", required_argument, 0, 'p'},
    {"rootfs", required_argument, 0, 'r'},
    {0, 0, 0, 0}
};

struct CmdArgs {
    char devices[BUF_SIZE];
    char rootfs[BUF_SIZE];
    int  pid;
};

struct ParsedConfig {
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

struct PathInfo {
    char* src;
    size_t srcLen;
    char* dst;
    size_t dstLen;
};

static inline bool IsCmdArgsValid(struct CmdArgs *args)
{
    return (strlen(args->devices) > 0) && (strlen(args->rootfs) > 0) && (args->pid > 0);
}

int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize)
{
    static const char *fmtStr = "/proc/%d/ns/%s";
    return snprintf_s(buf, BUF_SIZE, bufSize, fmtStr, pid, nsType);
}

int GetSelfNsPath(const char *nsType, char *buf, size_t bufSize)
{
    static const char *fmtStr = "/proc/self/ns/%s";
    return snprintf_s(buf, BUF_SIZE, bufSize, fmtStr, nsType);
}

int EnterNsByFd(int fd, int nsType)
{
    int ret = setns(fd, nsType);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set ns: fd(%d)\n", fd);
        return -1;
    }

    return 0;
}

int EnterNsByPath(const char *path, int nsType)
{
    int fd;
    int ret;

    fd = open(path, O_RDONLY); // proc文件接口，非外部输入
    if (fd < 0) {
        fprintf(stderr, "error: failed to open ns path: %s\n", path);
        return -1;
    }

    ret = EnterNsByFd(fd, nsType);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set ns: %s\n", path);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static int Mount(const char *src, const char *dst)
{
    static const unsigned long remountFlags = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID;
    int ret;

    ret = mount(src, dst, NULL, MS_BIND, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount\n");
        return -1;
    }

    ret = mount(NULL, dst, NULL, remountFlags, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to re-mount\n");
        return -1;
    }

    return 0;
}

static int CreateFile(const char *path, mode_t mode)
{
    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path, resolvedPath) == NULL && errno != ENOENT) {
        fprintf(stderr, "error: failed to resolve path %s\n", path);
        return -1;
    }

    int fd = open(resolvedPath, O_NOFOLLOW | O_CREAT, mode);
    if (fd < 0) {
        fprintf(stderr, "error: cannot create file: %s\n", resolvedPath);
        return -1;
    }
    close(fd);
    return 0;
}

static int VerfifyPathInfo(const struct PathInfo* pathInfo)
{
    if (pathInfo == NULL || pathInfo->dst == NULL || pathInfo->src == NULL) {
        return -1;
    }
    return 0;
}

static int GetDeviceMntSrcDst(const char *rootfs, const char *deviceName,
    struct PathInfo* pathInfo)
{
    int ret;
    error_t err;
    char unresolvedDst[BUF_SIZE] = {0};
    char resolvedDst[PATH_MAX] = {0};

    ret = VerfifyPathInfo(pathInfo);
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
        fprintf(stderr, "error: cannot canonicalize device dst: %s\n", dst);
        return -1;
    }

    err = strcpy_s(dst, dstBufSize, (const char *)resolvedDst);
    if (err != EOK) {
        fprintf(stderr, "error: failed to copy resolved device mnt path to dst: %s\n", resolvedDst);
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
        fprintf(stderr, "error: failed to get device mount src and(or) dst path, device name: %s\n", deviceName);
        return -1;
    }

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        fprintf(stderr, "error: failed to stat src: %s\n", src);
        return -1;
    }

    ret = CreateFile(dst, srcStat.st_mode);
    if (ret < 0) {
        fprintf(stderr, "error: failed to create mount dst file: %s\n", dst);
        return -1;
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount dev\n");
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

    token = strtok(list, sep);
    while (token != NULL) {
        int ret = snprintf_s(deviceName, BUF_SIZE, BUF_SIZE, "%s%s", DEVICE_NAME, token);
        if (ret < 0) {
            fprintf(stderr, "error: assemble device name failed, id: %s\n", token);
            return -1;
        }

        ret = MountDevice(rootfs, deviceName);
        if (ret < 0) {
            fprintf(stderr, "error: failed to mount device no. %s\n", token);
            return -1;
        }

        token = strtok(NULL, sep);
    }

    return 0;
}

int CheckDirExists(const char *dir)
{
    DIR *ptr = opendir(dir);
    if (NULL == ptr) {
        fprintf(stderr, "path %s not exist\n", dir);
        return -1;
    }

    fprintf(stdout, "path %s exist\n", dir);
    closedir(ptr);
    return 0;
}

int GetParentPathStr(const char *path, char *parent, size_t bufSize)
{
    char *ptr = strrchr(path, '/');
    if (ptr == NULL) {
        return 0;
    }

    int len = (int)strlen(path) - (int)strlen(ptr);
    if (len < 1) {
        return 0;
    }

    errno_t ret = strncpy_s(parent, bufSize, path, len);
    if (ret != EOK) {
        return -1;
    }

    return 0;
}

int MakeDir(const char *dir, int mode)
{
    return mkdir(dir, mode);
}

int MakeParentDir(const char *path, mode_t mode)
{
    if (*path == '\0' || *path == '.') {
        return 0;
    }
    if (CheckDirExists(path) == 0) {
        return 0;
    }

    char parentPath[BUF_SIZE] = {0};
    GetParentPathStr(path, parentPath, BUF_SIZE);
    if (strlen(parentPath) > 0 && MakeParentDir(parentPath, mode) < 0) {
        return -1;
    }

    struct stat s;
    int ret = stat(path, &s);
    if (ret < 0) {
        fprintf(stderr, "error: failed to stat path: %s\n", path);
        return (MakeDir(path, mode));
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
            fprintf(stderr, "error: failed to make dir: %s\n", parentDir);
            return -1;
        }
    }

    if (CheckDirExists(dst) < 0) {
        const mode_t curMode = srcStat.st_mode;
        ret = MakeDir(dst, curMode);
        if (ret < 0) {
            fprintf(stderr, "error: failed to make dir: %s\n", dst);
            return -1;
        }
    }

    ret = Mount(src, dst);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount dir: %s to %s\n", src, dst);
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    /* device */
    int ret = MountDevice(rootfs, DAVINCI_MANAGER);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount device %s\n", DAVINCI_MANAGER);
        return -1;
    }

    ret = MountDevice(rootfs, DEVMM_SVM);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount device %s\n", DEVMM_SVM);
        return -1;
    }

    ret = MountDevice(rootfs, HISI_HDC);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount device %s\n", HISI_HDC);
        return -1;
    }

    return 0;
}

int DoDirectoryMounting(const char *rootfs)
{
    /* directory */
    int ret = MountDir(rootfs, ASCEND_DRIVER_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", ASCEND_DRIVER_PATH);
        return -1;
    }

    ret = MountDir(rootfs, ASCEND_ADDONS_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", ASCEND_ADDONS_PATH);
        return -1;
    }

    return 0;
}

int DoMounting(const struct CmdArgs *args)
{
    int ret;

    ret = DoDeviceMounting(args->rootfs, args->devices);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mounts\n");
        return -1;
    }

    ret = DoCtrlDeviceMounting(args->rootfs);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount files\n");
        return -1;
    }

    ret = DoDirectoryMounting(args->rootfs);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount directory\n");
        return -1;
    }

    return 0;
}

typedef char *(*ParseFileLine)(char *, const char *);

int IsStrEqual(const char *s1, const char *s2)
{
    return (!strcmp(s1, s2));
}

int StrHasPrefix(const char *str, const char *prefix)
{
    return (!strncmp(str, prefix, strlen(prefix)));
}

static bool CheckRootDir(char **pLine)
{
    char *rootDir = NULL;
    for (int i = 0; i < ROOT_GAP; i++) {
        /* root is substr before gap, line is substr after gap */
        rootDir = strsep(pLine, " ");
        if (rootDir == NULL || *rootDir == '\0') {
            return false;
        }
    }

    return strlen(rootDir) < BUF_SIZE && !StrHasPrefix(rootDir, "/..");
}

static bool CheckFsType(char **pLine)
{
    char* fsType = NULL;
    for (int i = 0; i < FSTYPE_GAP; i++) {
        fsType = strsep(pLine, " ");
        if (fsType == NULL || *fsType == '\0') {
            return false;
        }
    }

    return IsStrEqual(fsType, "cgroup");
}

static bool CheckSubStr(char **pLine, const char *subsys)
{
    char* substr = NULL;
    for (int i = 0; i < MOUNT_SUBSTR_GAP; i++) {
        substr = strsep(pLine, " ");
        if (substr == NULL || *substr == '\0') {
            return false;
        }
    }

    return strstr(substr, subsys) != NULL;
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

int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath)
{
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    char resolvedPath[PATH_MAX] = {0x0};

    if (realpath(filepath, resolvedPath) == NULL && errno != ENOENT) {
        fprintf(stderr, "error: cannot canonicalize path %s\n", filepath);
        return -1;
    }

    fp = fopen(resolvedPath, "r");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file.\n");
        return -1;
    }

    while (getline(&line, &len, fp) != -1) {
        char* result = fn(line, "devices");
        if (result != NULL && strlen(result) < bufferSize) {
            errno_t ret = strncpy_s(buffer, BUF_SIZE, result, strlen(result));
            if (ret != EOK) {
                fclose(fp);
                return -1;
            }
            break;
        }
    }

    if (line != NULL) {
        free(line);
    }

    fclose(fp);
    return 0;
}

int SetupDeviceCgroup(FILE *cgroupAllow, const char *devName)
{
    int ret;
    struct stat devStat;
    char devPath[BUF_SIZE];

    ret = snprintf_s(devPath, BUF_SIZE, BUF_SIZE, "/dev/%s", devName);
    if (ret < 0) {
        fprintf(stderr, "error: failed to assemble dev path for %s\n", devName);
        return -1;
    }

    ret = stat((const char *)devPath, &devStat);
    if (ret < 0) {
        fprintf(stderr, "error: failed to get stat of %s\n", devPath);
        return -1;
    }

    bool isFailed = fprintf(cgroupAllow, "c %u:%u rw", major(devStat.st_rdev), minor(devStat.st_rdev)) < 0 ||
                    fflush(cgroupAllow) == EOF || ferror(cgroupAllow) < 0;
    if (isFailed) {
        fprintf(stderr, "error: write devices failed\n");
        return -1;
    }

    return 0; 
}

int SetupDriverCgroup(FILE *cgroupAllow)
{
    int ret;

    ret = SetupDeviceCgroup(cgroupAllow, DAVINCI_MANAGER);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", DAVINCI_MANAGER);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, DEVMM_SVM);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", DEVMM_SVM);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, HISI_HDC);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", HISI_HDC);
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
        fprintf(stderr, "error: assemble mount info path failed: ppid(%d)\n", getppid());
        return -1;
    }

    ret = CatFileContent(mount, BUF_SIZE, GetCgroupMount, mountPath);
    if (ret < 0) {
        fprintf(stderr, "error: cat file content failed\n");
        return -1;
    }

    char cgroup[BUF_SIZE] = {0x0};
    char cgroupPath[BUF_SIZE] = {0x0};
    ret = snprintf_s(cgroupPath, BUF_SIZE, BUF_SIZE, "/proc/%d/cgroup", args->pid);
    if (ret < 0) {
        fprintf(stderr, "error: assemble cgroup path failed: pid(%d)\n", args->pid);
        return -1;
    }

    ret = CatFileContent(cgroup, BUF_SIZE, GetCgroupRoot, cgroupPath);
    if (ret < 0) {
        fprintf(stderr, "error: cat file content failed\n");
        return -1;
    }

    // cut last '\n' off
    cgroup[strcspn(cgroup, "\n")] = '\0';

    ret = snprintf_s(effPath, BUF_SIZE, maxSize, "%s%s%s", mount, cgroup, ALLOW_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: assemble cgroup device path failed: \n");
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
        fprintf(stderr, "error: cannot canonicalize cgroup path: %s\n", cgroupPath);
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
        fprintf(stderr, "error: failed to open cgroup file: %s\n", resolvedCgroupPath);
        return -1;
    }

    ret = SetupDriverCgroup(cgroupAllow);
    if (ret < 0) {
        fclose(cgroupAllow);
        fprintf(stderr, "error: failed to setup driver cgroup\n");
        return -1;
    }
    
    token = strtok(list, sep);
    while (token != NULL) {
        ret = snprintf_s(deviceName, BUF_SIZE, BUF_SIZE, "%s%s", DEVICE_NAME, token);
        if (ret < 0) {
            fclose(cgroupAllow);
            fprintf(stderr, "error: failed to assemble device path for no.%s\n", token);
            return -1;
        }

        ret = SetupDeviceCgroup(cgroupAllow, (const char *)deviceName);
        if (ret < 0) {
            fclose(cgroupAllow);
            fprintf(stderr, "error: failed to setup cgroup %s\n", token);
            return -1;
        }

        token = strtok(NULL, sep);
    }

    fclose(cgroupAllow);
    return 0;
}

int DoPrepare(const struct CmdArgs *args, struct ParsedConfig *config)
{
    int ret;

    ret = GetNsPath(args->pid, "mnt", config->containerNsPath, BUF_SIZE);
    if (ret < 0) {
        fprintf(stderr, "error: failed to get container mnt ns path: pid(%d)\n", args->pid);
        return -1;
    }

    ret = GetCgroupPath(args, config->cgroupPath, BUF_SIZE);
    if (ret < 0) {
        fprintf(stderr, "error: failed to get cgroup path\n");
        return -1;
    }

    char originNsPath[BUF_SIZE] = {0};
    ret = GetSelfNsPath("mnt", originNsPath, BUF_SIZE);
    if (ret < 0) {
        fprintf(stderr, "error: failed to get self ns path\n");
        return -1;
    }

    config->originNsFd = open((const char *)originNsPath, O_RDONLY); // proc接口，非外部输入
    if (config->originNsFd < 0) {
        fprintf(stderr, "error: failed to get self ns fd: %s\n", originNsPath);
        return -1;
    }

    return 0;
}

int SetupMounts(struct CmdArgs *args)
{
    int ret;
    struct ParsedConfig config;

    ret = DoPrepare(args, &config);
    if (ret < 0) {
        fprintf(stderr, "error: failed to prepare nesessary config\n");
        return -1;
    }

    // enter container's mount namespace
    ret = EnterNsByPath((const char *)config.containerNsPath, CLONE_NEWNS);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set to container ns: %s\n", config.containerNsPath);
        close(config.originNsFd);
        return -1;
    }

    ret = DoMounting(args);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mounting\n");
        close(config.originNsFd);
        return -1;
    }

    ret = SetupCgroup(args, (const char *)config.cgroupPath);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set up cgroup\n");
        close(config.originNsFd);
        return -1;
    }

    // back to original namespace
    ret = EnterNsByFd(config.originNsFd, CLONE_NEWNS);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set ns back\n");
        close(config.originNsFd);
        return -1;
    }

    close(config.originNsFd);
    return 0;
}

int Process(int argc, char **argv)
{
    int c;
    errno_t err;
    int optionIndex;
    bool isSucceed;
    struct CmdArgs args = {0};

    isSucceed = true;
    while (isSucceed) {
        c = getopt_long(argc, argv, "d:p:r", g_cmdOpts, &optionIndex);
        if (c == -1) {
            // cmd options exhausted
            break;
        }

        switch (c) {
            case 'd':
                err = strcpy_s(args.devices, BUF_SIZE, optarg);
                if (err != EOK) {
                    isSucceed = false;
                }
                break;
            case 'p':
                args.pid = atoi(optarg);
                if (args.pid <= 0) {
                    isSucceed = false;
                }
                break;
            case 'r':
                err = strcpy_s(args.rootfs, BUF_SIZE, optarg);
                if (err != EOK) {
                    isSucceed = false;
                }
                break;
            default:
                fprintf(stderr, "unrecongnized option\n");
                isSucceed = false; // unrecognized option
                break;
        }
    }

    if (!isSucceed || !IsCmdArgsValid(&args)) {
        fprintf(stderr, "error: information not completed or valid.\n");
        return -1;
    }

    int ret = SetupMounts(&args);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

#ifdef gtest
int _main(int argc, char **argv)
{
#else
int main(int argc, char **argv)
{
#endif
    int ret = Process(argc, argv);
    return ret;
}
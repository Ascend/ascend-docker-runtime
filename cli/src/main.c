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
#include <unistd.h>
#include <sched.h>
#include <dirent.h>

#define DEVICE_NAME "davinci"
#define DAVINCI_MANAGER_PATH        "/dev/davinci_manager"
#define DEVMM_SVM_PATH              "/dev/devmm_svm"
#define HISI_HDC_PATH               "/dev/hisi_hdc"
#define ASCEND_DRIVER_PATH          "/usr/local/Ascend/driver"
#define DEFAULT_DIR_MODE 0755
#define BUF_SIZE 1024
#define ALLOW_PATH "/devices.allow"
#define ROOT_GAP 4
#define FSTYPE_GAP 2
#define MOUNT_SUBSTR_GAP 2
#define ROOT_SUBSTR_GAP 2

static const struct option g_opts[] = {
    {"devices", required_argument, 0, 'd'},
    {"pid", required_argument, 0, 'p'},
    {"rootfs", required_argument, 0, 'r'},
    {0, 0, 0, 0}
};

struct CmdArgs {
    char *devices;
    char *rootfs;
    int  pid;
};

struct ParsedConfig {
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

static inline bool IsCmdArgsValid(struct CmdArgs *args)
{
    return (args->devices != NULL) && (args->rootfs != NULL) && (args->pid > 0);
}

void FreeCmdArgs(struct CmdArgs *args)
{
    if (args->devices != NULL) {
        free(args->devices);
        args->devices = NULL;
    }

    if (args->rootfs != NULL) {
        free(args->rootfs);
        args->rootfs = NULL;
    }

    args->pid = -1;
}

int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize)
{
    static const char *fmtStr = "/proc/%d/ns/%s";
    return snprintf(buf, bufSize, fmtStr, pid, nsType);
}

int GetSelfNsPath(const char *nsType, char *buf, size_t bufSize)
{
    static const char *fmtStr = "/proc/self/ns/%s";
    return snprintf(buf, bufSize, fmtStr, nsType);
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

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "error: failed to open ns path: %s\n", path);
        return -1;
    }

    ret = EnterNsByFd(fd, nsType);
    if (ret < 0) {
        fprintf(stderr, "error: failed to set ns: %s\n", path);
        return -1;
    }

    close(fd);
    return 0;
}

unsigned int GetNextSerialNum()
{
    static unsigned int index = 0;

    return index++;
}

int MountDevice(const char *rootfs, const int serialNumber)
{
    int ret;
    char src[BUF_SIZE] = {0};
    char dst[BUF_SIZE] = {0};
    char tmp[BUF_SIZE] = {0};

    snprintf(src, BUF_SIZE, "/dev/" DEVICE_NAME "%d", serialNumber);
    unsigned int targetSerialDevId = GetNextSerialNum();
    snprintf(tmp, BUF_SIZE, "/dev/" DEVICE_NAME "%d", targetSerialDevId);
    snprintf(dst, BUF_SIZE, "%s%s", rootfs, (const char *)tmp);

    struct stat srcStat;
    ret = stat((const char *)src, &srcStat);
    if (ret < 0) {
        fprintf(stderr, "error: failed to stat src: %s\n", src);
        return -1;
    }

    close(open(dst, O_NOFOLLOW | O_CREAT, srcStat.st_mode));

    ret = mount(src, dst, NULL, MS_BIND, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount dev\n");
        return -1;
    }

    ret = mount(NULL, dst, NULL, MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to re-mount dev\n");
        return -1;
    }

    return 0;
}

int DoDeviceMounting(const char *rootfs, const char *devicesList)
{
    static const char *sep = ",";
    char list[BUF_SIZE] = {0};
    strcpy(list, devicesList);
    char *token = NULL;

    token = strtok(list, sep);
    while (token != NULL) {
        int ret = MountDevice(rootfs, atoi((const char *) token));
        if (ret < 0) {
            fprintf(stderr, "error: failed to mount device no. %s\n", token);
            return -1;
        }

        token = strtok(NULL, sep);
    }

    return 0;
}

int CheckDirExists(char *dir, int len)
{
    if (len < 0) {
        fprintf(stderr, "length of path is %d\n", len);
        return -1;
    }
    DIR *ptr = opendir(dir);
    if (NULL == ptr) {
        fprintf(stderr, "path %s not exist\n", dir);
        return -1;
    } else {
        fprintf(stdout, "path %s exist\n", dir);
        closedir(ptr);
        return 0;
    }
}

int GetParentPathStr(const char *path, int lenOfPath, char *parent)
{
    if (lenOfPath < 0) {
        return -1;
    }
    char *ptr = strrchr(path, '/');
    if (ptr == NULL) {
        return 0;
    }
    int len = strlen(path) - strlen(ptr);
    if (len < 1) {
        return 0;
    }
    strncpy(parent, path, len);
    return 0;
}

int MakeDir(char *dir, int mode)
{
    return mkdir(dir, mode);
}

int MakeParentDir(char *path, mode_t mode)
{
    if (*path == '\0' || *path == '.') {
        return 0;
    }
    if (CheckDirExists(path, strlen(path)) == 0) {
        return 0;
    }
    char parentPath[BUF_SIZE] = {0};
    GetParentPathStr(path, strlen(path), parentPath);
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

int MountFiles(const char *rootfs, const char *file, unsigned long reMountRwFlag)
{
    char src[BUF_SIZE] = {0};
    char dst[BUF_SIZE] = {0};
    snprintf(src, BUF_SIZE, "%s", file);
    snprintf(dst, BUF_SIZE, "%s%s", rootfs, file);

    struct stat srcStat;
    int ret = stat((const char *) src, &srcStat);
    if (ret < 0) {
        fprintf(stderr, "error: failed to stat src: %s\n", src);
        return -1;
    }

    if (S_ISDIR(srcStat.st_mode)) {
        /* directory */
        char parentDir[BUF_SIZE] = {0};
        GetParentPathStr(dst, strlen(dst), parentDir);
        if (CheckDirExists(parentDir, strlen(parentDir)) < 0) {
            mode_t parentMode = DEFAULT_DIR_MODE;
            ret = MakeParentDir(parentDir, parentMode);
            if (ret < 0) {
                fprintf(stderr, "error: failed to make dir: %s\n", parentDir);
                return -1;
            }
        }
        if (CheckDirExists(dst, strlen(dst)) < 0) {
            const mode_t curMode = srcStat.st_mode;
            ret = MakeDir(dst, curMode);
            if (ret < 0) {
                fprintf(stderr, "error: failed to make dir: %s\n", dst);
                return -1;
            }
        }
    } else {
        /* device */
        close(open(dst, O_NOFOLLOW | O_CREAT, srcStat.st_mode));
    }

    ret = mount(src, dst, NULL, MS_BIND, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to mount dev\n");
        return -1;
    }

    ret = mount(NULL, dst, NULL, reMountRwFlag, NULL);
    if (ret < 0) {
        fprintf(stderr, "error: failed to re-mount dev\n");
        return -1;
    }

    return 0;
}

int DoCtrlDeviceMounting(const char *rootfs)
{
    /* device */
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountFiles(rootfs, DAVINCI_MANAGER_PATH, reMountRwFlag);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", DAVINCI_MANAGER_PATH);
        return -1;
    }
    ret = MountFiles(rootfs, DEVMM_SVM_PATH, reMountRwFlag);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", DEVMM_SVM_PATH);
        return -1;
    }
    ret = MountFiles(rootfs, HISI_HDC_PATH, reMountRwFlag);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", HISI_HDC_PATH);
        return -1;
    }
    return 0;
}

int DoDirectoryMounting(const char *rootfs)
{
    /* directory */
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NODEV | MS_NOSUID;
    int ret = MountFiles(rootfs, ASCEND_DRIVER_PATH, reMountRwFlag);
    if (ret < 0) {
        fprintf(stderr, "error: failed to do mount %s\n", ASCEND_DRIVER_PATH);
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

static bool IsCgroupLineArgsValid(const char *rootDir, const char *mountPoint, const char* fsType, const char* substr)
{
    return ((rootDir != NULL && mountPoint != NULL && fsType != NULL && substr != NULL) &&
           (*rootDir != '\0' && *mountPoint != '\0' && *fsType != '\0' && *substr != '\0'));
}

char *GetCgroupMount(char *line, const char *subsys)
{
    int i;

    char *rootDir = NULL;
    for (i = 0; i < ROOT_GAP; ++i) {
        /* root is substr before gap, line is substr after gap */
        rootDir = strsep(&line, " ");
    }

    char *mountPoint = NULL;
    mountPoint = strsep(&line, " ");
    line = strchr(line, '-');

    char* fsType = NULL;
    for (i = 0; i < FSTYPE_GAP; ++i) {
        fsType = strsep(&line, " ");
    }

    char* substr = NULL;
    for (i = 0; i < MOUNT_SUBSTR_GAP; ++i) {
        substr = strsep(&line, " ");
    }

    if (!IsCgroupLineArgsValid(rootDir, mountPoint, fsType, substr)) {
        return NULL;
    }

    if (strlen(rootDir) >= BUF_SIZE || StrHasPrefix(rootDir, "/..")) {
        return NULL;
    }

    if (strstr(substr, subsys) == NULL) {
        return NULL;
    }

    if (!IsStrEqual(fsType, "cgroup")) {
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
    }
    char *rootDir = strsep(&line, ":");
    if (rootDir == NULL || token == NULL) {
        return (NULL);
    }
    if (*rootDir == '\0' || *token == '\0') {
        return (NULL);
    }
    if (strlen(rootDir) >= BUF_SIZE || StrHasPrefix(rootDir, "/..")) {
        return (NULL);
    }
    if (strstr(token, subSystem) == NULL) {
        return (NULL);
    }
    return (rootDir);
}

int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;

    fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file.\n");
        return -1;
    }

    while (getline(&line, &len, fp) != -1) {
        char* result = fn(line, "devices");
        if (result != NULL && strlen(result) < bufferSize) {
            strncpy(buffer, result, strlen(result));
            break;
        }
    }

    if (line != NULL) {
        free(line);
    }

    fclose(fp);
    return 0;
}

int SetupDeviceCgroup(FILE *cgroupAllow, const char *devPath)
{
    int ret;
    struct stat devStat;
    ret = stat(devPath, &devStat);
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

    ret = SetupDeviceCgroup(cgroupAllow, DAVINCI_MANAGER_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", DAVINCI_MANAGER_PATH);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, DEVMM_SVM_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", DEVMM_SVM_PATH);
        return -1;
    }

    ret = SetupDeviceCgroup(cgroupAllow, HISI_HDC_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: failed to setup cgroup for %s\n", HISI_HDC_PATH);
        return -1;
    }

    return 0;
}

int GetCgroupPath(const struct CmdArgs *args, char *effPath, const size_t maxSize)
{
    int ret;
    char mountPath[BUF_SIZE] = {0x0};
    char mount[BUF_SIZE] = {0x0};

    ret = snprintf(mountPath, BUF_SIZE, "/proc/%d/mountinfo", (int)getppid());
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
    ret = snprintf(cgroupPath, BUF_SIZE, "/proc/%d/cgroup", args->pid);
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

    ret = snprintf(effPath, maxSize, "%s%s%s", mount, cgroup, ALLOW_PATH);
    if (ret < 0) {
        fprintf(stderr, "error: assemble cgroup device path failed: \n");
        return -1;
    }

    return 0;
}

int SetupCgroup(struct CmdArgs *args, const char *cgroupPath)
{
    int ret;
    char devicePath[BUF_SIZE] = {0};
    FILE *cgroupAllow = NULL;

    static const char *sep = ",";
    char list[BUF_SIZE] = {0};
    strcpy(list, args->devices);
    char *token = NULL;

    cgroupAllow = fopen(cgroupPath, "a");
    if (cgroupAllow == NULL) {
        fprintf(stderr, "error: failed to open cgroup file: %s\n", cgroupPath);
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
        ret = snprintf(devicePath, BUF_SIZE, "/dev/" DEVICE_NAME "%d", atoi(token));
        if (ret < 0) {
            fclose(cgroupAllow);
            fprintf(stderr, "error: failed to assemble device path for no.%s\n", token);
            return -1;
        }

        ret = SetupDeviceCgroup(cgroupAllow, (const char *)devicePath);
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

    config->originNsFd = open((const char *)originNsPath, O_RDONLY);
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
    int optionIndex;
    struct CmdArgs args = {
        .devices = NULL,
        .rootfs  = NULL,
        .pid     = -1
    };

    while ((c = getopt_long(argc, argv, "d:p:r", g_opts, &optionIndex)) != -1) {
        switch (c) {
            case 'd':
                args.devices = strdup(optarg);
                break;
            case 'p':
                args.pid = atoi(optarg);
                break;
            case 'r':
                args.rootfs = strdup(optarg);
                break;
            default:
                fprintf(stderr, "unrecongnized option\n");
                return -1; // unrecognized option
        }
    }

    if (!IsCmdArgsValid(&args)) {
        FreeCmdArgs(&args);
        return -1;
    }

    int ret = SetupMounts(&args);
    if (ret < 0) {
        FreeCmdArgs(&args);
        return ret;
    }

    FreeCmdArgs(&args);
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
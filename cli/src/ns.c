#define _GNU_SOURCE
#include "ns.h"

#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include "basic.h"
#include "securec.h"
#include "logging.h"

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
        logError("error: failed to set ns: fd(%d)\n", fd);
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
        logError("error: failed to open ns path: %s\n", path);
        return -1;
    }

    ret = EnterNsByFd(fd, nsType);
    if (ret < 0) {
        logError("error: failed to set ns: %s\n", path);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
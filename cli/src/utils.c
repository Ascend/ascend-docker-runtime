#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include "securec.h"
#include "logging.h"

int IsStrEqual(const char *s1, const char *s2)
{
    return (!strcmp(s1, s2));
}

int StrHasPrefix(const char *str, const char *prefix)
{
    return (!strncmp(str, prefix, strlen(prefix)));
}

bool CheckRootDir(char **pLine)
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

bool CheckFsType(char **pLine)
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

bool CheckSubStr(char **pLine, const char *subsys)
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

int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath)
{
    FILE *fp = NULL;
    char *line = NULL;
    size_t len = 0;
    char resolvedPath[PATH_MAX] = {0x0};

    if (realpath(filepath, resolvedPath) == NULL && errno != ENOENT) {
        logError("error: cannot canonicalize path %s\n", filepath);
        return -1;
    }

    fp = fopen(resolvedPath, "r");
    if (fp == NULL) {
        logError("cannot open file.\n");
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

int MkDir(const char *dir, int mode)
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
        logError("error: failed to stat path: %s\n", path);
        return (MkDir(path, mode));
    }

    return 0;
}

int CheckDirExists(const char *dir)
{
    DIR *ptr = opendir(dir);
    if (NULL == ptr) {
        logError("path %s not exist\n", dir);
        return -1;
    }

    logInfo("path %s exist\n", dir);
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

int CreateFile(const char *path, mode_t mode)
{
    char resolvedPath[PATH_MAX] = {0};
    if (realpath(path, resolvedPath) == NULL && errno != ENOENT) {
        logError("error: failed to resolve path %s\n", path);
        return -1;
    }

    int fd = open(resolvedPath, O_NOFOLLOW | O_CREAT, mode);
    if (fd < 0) {
        logError("error: cannot create file: %s\n", resolvedPath);
        return -1;
    }
    close(fd);
    return 0;
}

int VerfifyPathInfo(const struct PathInfo* pathInfo)
{
    if (pathInfo == NULL || pathInfo->dst == NULL || pathInfo->src == NULL) {
        return -1;
    }
    return 0;
}

int Mount(const char *src, const char *dst)
{
    static const unsigned long remountFlags = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID;
    int ret;

    ret = mount(src, dst, NULL, MS_BIND, NULL);
    if (ret < 0) {
        logError("error: failed to mount\n");
        return -1;
    }

    ret = mount(NULL, dst, NULL, remountFlags, NULL);
    if (ret < 0) {
        logError("error: failed to re-mount\n");
        return -1;
    }

    return 0;
}
#ifndef _NS_H
#define _NS_H

#include <sys/types.h>

int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
int GetSelfNsPath(const char *nsType, char *buf, size_t bufSize);
int EnterNsByFd(int fd, int nsType);
int EnterNsByPath(const char *path, int nsType);

#endif
#ifndef _UTILS_H
#define _UTILS_H

#include <stdbool.h>
#include <limits.h>
#include <sys/types.h>
#include "basic.h"

typedef char *(*ParseFileLine)(char *, const char *);
int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath);
bool CheckFsType(char **pLine);
bool CheckSubStr(char **pLine, const char *subsys);
bool CheckRootDir(char **pLine);
int MakeDir(const char *dir, int mode);
int CheckDirExists(const char *dir);
int GetParentPathStr(const char *path, char *parent, size_t bufSize);
int CreateFile(const char *path, mode_t mode);
int VerfifyPathInfo(const struct PathInfo* pathInfo);
int Mount(const char *src, const char *dst);
int IsStrEqual(const char *s1, const char *s2);
int StrHasPrefix(const char *str, const char *prefix);

#endif
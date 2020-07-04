#ifndef _CGRP_H
#define _CGRP_H

#include "basic.h"

int GetCgroupPath(const struct CmdArgs *args, char *effPath, size_t maxSize);
int SetupCgroup(struct CmdArgs *args, const char *cgroupPath);

#endif
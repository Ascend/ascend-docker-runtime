/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具，配置容器挂载Ascend NPU设备
*/
#define _GNU_SOURCE
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include "securec.h"

#include "basic.h"
#include "ns.h"
#include "mount.h"
#include "cgrp.h"
#include "logging.h"

struct ParsedConfig {
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

static struct option g_cmdOpts[] = {
    {"devices", required_argument, 0, 'd'},
    {"pid", required_argument, 0, 'p'},
    {"rootfs", required_argument, 0, 'r'},
    {0, 0, 0, 0}
};

static inline bool IsCmdArgsValid(struct CmdArgs *args)
{
    return (strlen(args->devices) > 0) && (strlen(args->rootfs) > 0) && (args->pid > 0);
}

int DoPrepare(const struct CmdArgs *args, struct ParsedConfig *config)
{
    int ret;

    ret = GetNsPath(args->pid, "mnt", config->containerNsPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get container mnt ns path: pid(%d)\n", args->pid);
        return -1;
    }

    ret = GetCgroupPath(args, config->cgroupPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get cgroup path\n");
        return -1;
    }

    char originNsPath[BUF_SIZE] = {0};
    ret = GetSelfNsPath("mnt", originNsPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get self ns path\n");
        return -1;
    }

    config->originNsFd = open((const char *)originNsPath, O_RDONLY); // proc接口，非外部输入
    if (config->originNsFd < 0) {
        LogError("error: failed to get self ns fd: %s\n", originNsPath);
        return -1;
    }

    return 0;
}

int SetupContainer(struct CmdArgs *args)
{
    int ret;
    struct ParsedConfig config;

    ret = DoPrepare(args, &config);
    if (ret < 0) {
        LogError("error: failed to prepare nesessary config\n");
        return -1;
    }

    // enter container's mount namespace
    ret = EnterNsByPath((const char *)config.containerNsPath, CLONE_NEWNS);
    if (ret < 0) {
        LogError("error: failed to set to container ns: %s\n", config.containerNsPath);
        close(config.originNsFd);
        return -1;
    }

    ret = DoMounting(args);
    if (ret < 0) {
        LogError("error: failed to do mounting\n");
        close(config.originNsFd);
        return -1;
    }

    ret = SetupCgroup(args, (const char *)config.cgroupPath);
    if (ret < 0) {
        LogError("error: failed to set up cgroup\n");
        close(config.originNsFd);
        return -1;
    }

    // back to original namespace
    ret = EnterNsByFd(config.originNsFd, CLONE_NEWNS);
    if (ret < 0) {
        LogError("error: failed to set ns back\n");
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
                LogError("unrecongnized option\n");
                isSucceed = false; // unrecognized option
                break;
        }
    }

    if (!isSucceed || !IsCmdArgsValid(&args)) {
        LogError("error: information not completed or valid.\n");
        return -1;
    }

    int ret = SetupContainer(&args);
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
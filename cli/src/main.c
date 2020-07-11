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
#include "options.h"

#define DECIMAL     10

struct ParsedConfig {
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

static struct option g_cmdOpts[] = {
    {"devices", required_argument, 0, 'd'},
    {"pid", required_argument, 0, 'p'},
    {"rootfs", required_argument, 0, 'r'},
    {"options", required_argument, 0, 'o'},
    {0, 0, 0, 0}
};

typedef bool (*CmdArgParser)(struct CmdArgs *args, const char *arg);

static bool DevicesCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->devices, BUF_SIZE, arg);
    if (err != EOK) {
        LogError("error: failed to get devices from cmd args.");
        return false;
    }

    return true;
}

static bool PidCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno = 0;
    args->pid = strtol(optarg, NULL, DECIMAL);
    if (errno != 0) {
        LogError("error: failed to convert pid string from cmd args, pid string: %s.", arg);
        return false;
    }

    if (args->pid <= 0) {
        LogError("error: invalid pid %d.", args->pid);
        return false;
    }

    return true;
}

static bool RootfsCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->rootfs, BUF_SIZE, arg);
    if (err != EOK) {
        LogError("error: failed to get rootfs path from cmd args");
        return false;
    }

    return true;
}

static bool OptionsCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->options, BUF_SIZE, arg);
    if (err != EOK) {
        LogError("error: failed to get options string from cmd args");
        return false;
    }

    return true;
}

#define NUM_OF_CMD_ARGS 4

static struct {
    const int c;
    CmdArgParser parser;
} g_cmdArgParsers[NUM_OF_CMD_ARGS] = {
    {'d', DevicesCmdArgParser},
    {'p', PidCmdArgParser},
    {'r', RootfsCmdArgParser},
    {'o', OptionsCmdArgParser}
};

static int ParseOneCmdArg(struct CmdArgs *args, int indicator, const char *value)
{
    int i;
    for (i = 0; i < NUM_OF_CMD_ARGS; i++) {
        if (g_cmdArgParsers[i].c == indicator) {
            break;
        }
    }

    if (i == NUM_OF_CMD_ARGS) {
        LogError("error: unrecognized cmd arg: indicate char: %c, value: %s.", indicator, value);
        return -1;
    }

    bool isOK = g_cmdArgParsers[i].parser(args, value);
    if (!isOK) {
        LogError("error: failed while parsing cmd arg, indicate char: %c, value: %s.", indicator, value);
        return -1;
    }

    return 0;
}

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

    ret = ParseRuntimeOptions(args->options);
    if (ret < 0) {
        LogError("error: failed to parse runtime options.");
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
    int ret;
    int optionIndex;
    struct CmdArgs args = {0};

    while ((c = getopt_long(argc, argv, "d:p:r:o", g_cmdOpts, &optionIndex)) != -1) {
        ret = ParseOneCmdArg(&args, c, optarg);
        if (ret < 0) {
            LogError("error: failed to parse cmd args.");
            return -1;
        }
    }

    if (!IsCmdArgsValid(&args)) {
        LogError("error: information not completed or valid.\n");
        return -1;
    }

    ret = SetupContainer(&args);
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
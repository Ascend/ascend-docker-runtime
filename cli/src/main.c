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

struct CmdArgs {
    char     devices[BUF_SIZE];
    char     rootfs[BUF_SIZE];
    int      pid;
    char     options[BUF_SIZE];
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

static int ParseDeviceIDs(unsigned int *idList, size_t *idListSize, char *devices)
{
    static const char *sep = ",";
    char *token = NULL;
    char *context = NULL;
    size_t idx = 0;

    token = strtok_s(devices, sep, &context);
    while (token != NULL) {
        if (idx >= *idListSize) {
            LogError("error: too many devices(%u), support %u devices maximally", idx, *idListSize);
            return -1;
        }

        errno = 0;
        idList[idx] = strtoul((const char *)token, NULL, DECIMAL);
        if (errno != 0) {
            LogError("error: failed to convert device id (%s) from cmd args, caused by: %s.", token, strerror(errno));
            return -1;
        }

        idx++;
        token = strtok_s(NULL, sep, &context);
    }

    *idListSize = idx;
    return 0;
}

int DoPrepare(const struct CmdArgs *args, struct ParsedConfig *config)
{
    int ret;
    errno_t err;

    err = strcpy_s(config->rootfs, BUF_SIZE, args->rootfs);
    if (err != EOK) {
        LogError("error: failed to copy rootfs path to parsed config.");
        return -1;
    }

    ret = ParseDeviceIDs(config->devices, &config->devicesNr, (char *)args->devices);
    if (ret < 0) {
        LogError("error: failed to parse device ids from cmdline argument");
        return -1;
    }

    ret = GetNsPath(args->pid, "mnt", config->containerNsPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get container mnt ns path: pid(%d).", args->pid);
        return -1;
    }

    ret = GetCgroupPath(args->pid, config->cgroupPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get cgroup path.");
        return -1;
    }

    char originNsPath[BUF_SIZE] = {0};
    ret = GetSelfNsPath("mnt", originNsPath, BUF_SIZE);
    if (ret < 0) {
        LogError("error: failed to get self ns path.");
        return -1;
    }

    config->originNsFd = open((const char *)originNsPath, O_RDONLY); // proc接口，非外部输入
    if (config->originNsFd < 0) {
        LogError("error: failed to get self ns fd: %s.", originNsPath);
        return -1;
    }

    return 0;
}

int SetupContainer(struct CmdArgs *args)
{
    int ret;
    struct ParsedConfig config;

    InitParsedConfig(&config);

    ret = DoPrepare(args, &config);
    if (ret < 0) {
        LogError("error: failed to prepare nesessary config.");
        return -1;
    }

    // enter container's mount namespace
    ret = EnterNsByPath((const char *)config.containerNsPath, CLONE_NEWNS);
    if (ret < 0) {
        LogError("error: failed to set to container ns: %s.", config.containerNsPath);
        close(config.originNsFd);
        return -1;
    }

    ret = DoMounting(&config);
    if (ret < 0) {
        LogError("error: failed to do mounting.");
        close(config.originNsFd);
        return -1;
    }

    ret = SetupCgroup(&config);
    if (ret < 0) {
        LogError("error: failed to set up cgroup.");
        close(config.originNsFd);
        return -1;
    }

    // back to original namespace
    ret = EnterNsByFd(config.originNsFd, CLONE_NEWNS);
    if (ret < 0) {
        LogError("error: failed to set ns back.");
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
        LogError("error: information not completed or valid.");
        return -1;
    }

    ret = ParseRuntimeOptions(args.options);
    if (ret < 0) {
        LogError("error: failed to parse runtime options.");
        return -1;
    }

    SetPidForLog(args.pid);

    ret = OpenLog(DEFAULT_LOG_FILE);
    if (ret < 0) {
        LogError("error: failed to open log file %s.", DEFAULT_LOG_FILE);
        return -1;
    }

    ret = SetupContainer(&args);
    if (ret < 0) {
        CloseLog();
        return ret;
    }

    CloseLog();

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
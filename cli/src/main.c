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
#include "u_mount.h"
#include "cgrp.h"
#include "options.h"

#define DECIMAL     10

struct CmdArgs {
    char     devices[BUF_SIZE];
    char     rootfs[BUF_SIZE];
    int      pid;
    char     options[BUF_SIZE];
    struct MountList files;
    struct MountList dirs;
};

static struct option g_cmdOpts[] = {
    {"devices", required_argument, 0, 'd'},
    {"pid", required_argument, 0, 'p'},
    {"rootfs", required_argument, 0, 'r'},
    {"options", required_argument, 0, 'o'},
    {"mount-file", required_argument, 0, 'f'},
    {"mount-dir", required_argument, 0, 'i'},
    {0, 0, 0, 0}
};

typedef bool (*CmdArgParser)(struct CmdArgs *args, const char *arg);

static bool DevicesCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->devices, BUF_SIZE, arg);
    if (err != EOK) {
        LOG_ERROR("error: failed to get devices from cmd args.");
        return false;
    }

    return true;
}

static bool PidCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno = 0;
    args->pid = strtol(optarg, NULL, DECIMAL);
    if (errno != 0) {
        LOG_ERROR("error: failed to convert pid string from cmd args, pid string: %s.", arg);
        return false;
    }

    if (args->pid <= 0) {
        LOG_ERROR("error: invalid pid %d.", args->pid);
        return false;
    }

    return true;
}

static bool RootfsCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->rootfs, BUF_SIZE, arg);
    if (err != EOK) {
        LOG_ERROR("error: failed to get rootfs path from cmd args");
        return false;
    }

    return true;
}

static bool OptionsCmdArgParser(struct CmdArgs *args, const char *arg)
{
    errno_t err = strcpy_s(args->options, BUF_SIZE, arg);
    if (err != EOK) {
        LOG_ERROR("error: failed to get options string from cmd args");
        return false;
    }

    return true;
}

static bool MountFileCmdArgParser(struct CmdArgs *args, const char *arg)
{
    if (args->files.count == MAX_MOUNT_NR) {
        LOG_ERROR("error: too many files to mount, max number is %u", MAX_MOUNT_NR);
        return -1;
    }

    char *dst = &args->files.list[args->files.count++][0];
    errno_t err = strcpy_s(dst, PATH_MAX, arg);
    if (err != EOK) {
        LOG_ERROR("error: failed to copy mount file path: %s", arg);
        return false;
    }

    return true;
}

static bool MountDirCmdArgParser(struct CmdArgs *args, const char *arg)
{
    if (args->dirs.count == MAX_MOUNT_NR) {
        LOG_ERROR("error: too many directories to mount, max number is %u", MAX_MOUNT_NR);
        return -1;
    }

    char *dst = &args->dirs.list[args->dirs.count++][0];
    errno_t  err = strcpy_s(dst, PATH_MAX, arg);
    if (err != EOK) {
        LOG_ERROR("error: failed to copy mount directory path: %s", arg);
        return false;
    }

    return true;
}

#define NUM_OF_CMD_ARGS 6

static struct {
    const char c;
    CmdArgParser parser;
} g_cmdArgParsers[NUM_OF_CMD_ARGS] = {
    {'d', DevicesCmdArgParser},
    {'p', PidCmdArgParser},
    {'r', RootfsCmdArgParser},
    {'o', OptionsCmdArgParser},
    {'f', MountFileCmdArgParser},
    {'i', MountDirCmdArgParser}
};

static int ParseOneCmdArg(struct CmdArgs *args, char indicator, const char *value)
{
    int i;
    for (i = 0; i < NUM_OF_CMD_ARGS; i++) {
        if (g_cmdArgParsers[i].c == indicator) {
            break;
        }
    }

    if (i == NUM_OF_CMD_ARGS) {
        LOG_ERROR("error: unrecognized cmd arg: indicate char: %c, value: %s.", indicator, value);
        return -1;
    }

    bool isOK = g_cmdArgParsers[i].parser(args, value);
    if (!isOK) {
        LOG_ERROR("error: failed while parsing cmd arg, indicate char: %c, value: %s.", indicator, value);
        return -1;
    }

    return 0;
}

static inline bool IsCmdArgsValid(const struct CmdArgs *args)
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
            LOG_ERROR("error: too many devices(%u), support %u devices maximally", idx, *idListSize);
            return -1;
        }

        errno = 0;
        idList[idx] = strtoul((const char *)token, NULL, DECIMAL);
        if (errno != 0) {
            LOG_ERROR("error: failed to convert device id (%s) from cmd args", token);
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
        LOG_ERROR("error: failed to copy rootfs path to parsed config.");
        return -1;
    }

    ret = ParseDeviceIDs(config->devices, &config->devicesNr, (char *)args->devices);
    if (ret < 0) {
        LOG_ERROR("error: failed to parse device ids from cmdline argument");
        return -1;
    }

    ret = GetNsPath(args->pid, "mnt", config->containerNsPath, BUF_SIZE);
    if (ret < 0) {
        LOG_ERROR("error: failed to get container mnt ns path: pid(%d).", args->pid);
        return -1;
    }

    ret = GetCgroupPath(args->pid, config->cgroupPath, BUF_SIZE);
    if (ret < 0) {
        LOG_ERROR("error: failed to get cgroup path.");
        return -1;
    }

    char originNsPath[BUF_SIZE] = {0};
    ret = GetSelfNsPath("mnt", originNsPath, BUF_SIZE);
    if (ret < 0) {
        LOG_ERROR("error: failed to get self ns path.");
        return -1;
    }

    config->originNsFd = open((const char *)originNsPath, O_RDONLY); // proc接口，非外部输入
    if (config->originNsFd < 0) {
        LOG_ERROR("error: failed to get self ns fd: %s.", originNsPath);
        return -1;
    }

    config->files = (const struct MountList *)&args->files;
    config->dirs  = (const struct MountList *)&args->dirs;

    return 0;
}

int SetupContainer(struct CmdArgs *args)
{
    int ret;
    struct ParsedConfig config;

    InitParsedConfig(&config);

    ret = DoPrepare(args, &config);
    if (ret < 0) {
        LOG_ERROR("error: failed to prepare nesessary config.");
        return -1;
    }

    // enter container's mount namespace
    ret = EnterNsByPath((const char *)config.containerNsPath, CLONE_NEWNS);
    if (ret < 0) {
        LOG_ERROR("error: failed to set to container ns: %s.", config.containerNsPath);
        close(config.originNsFd);
        return -1;
    }

    ret = DoMounting(&config);
    if (ret < 0) {
        LOG_ERROR("error: failed to do mounting.");
        close(config.originNsFd);
        return -1;
    }

    ret = SetupCgroup(&config);
    if (ret < 0) {
        LOG_ERROR("error: failed to set up cgroup.");
        close(config.originNsFd);
        return -1;
    }

    // back to original namespace
    ret = EnterNsByFd(config.originNsFd, CLONE_NEWNS);
    if (ret < 0) {
        LOG_ERROR("error: failed to set ns back.");
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

    while ((c = getopt_long(argc, argv, "d:p:r:o:f:i", g_cmdOpts, &optionIndex)) != -1) {
        ret = ParseOneCmdArg(&args, (char)c, optarg);
        if (ret < 0) {
            LOG_ERROR("error: failed to parse cmd args.");
            return -1;
        }
    }

    if (!IsCmdArgsValid(&args)) {
        LOG_ERROR("error: information not completed or valid.");
        return -1;
    }

    ParseRuntimeOptions(args.options);

    ret = SetupContainer(&args);
    if (ret < 0) {
        LOG_ERROR("error: failed to setup container.");
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
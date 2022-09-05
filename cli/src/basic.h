/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#ifndef _BASIC_H
#define _BASIC_H

#include <sys/types.h>
#include <stdint.h>
#include <limits.h>

#define DEVICE_NAME           "davinci"
#define VDEVICE_NAME          "vdavinci"
#define DAVINCI_MANAGER       "davinci_manager"
#define DEVMM_SVM             "devmm_svm"
#define HISI_HDC              "hisi_hdc"
#define DEFAULT_DIR_MODE      0755
#define DEFAULT_LOG_MODE      0600
#define DUMP_LOG_MODE         0400
#define DEFAULT_LOGDIR_MODE   0700
#define BUF_SIZE              1024
#define MAX_DEVICE_NR         1024
#define MAX_MOUNT_NR          512
#define WHITE_LIST_NUM        9
 
#define ROOT_UID              0

#define ROOT_UID              0

#define LEVEL_INFO     0
#define LEVEL_WARN     1
#define LEVEL_ERROR    2
#define LEVEL_DEBUG    3
#define SCREEN_NO      0
#define SCREEN_YES     1

#define LOG_ERROR(fmt, ...)                                           \
    do {                                                              \
        char _content[BUF_SIZE] = {0};                                \
        int _ret = sprintf_s(_content, BUF_SIZE, fmt, ##__VA_ARGS__); \
        if (_ret < 0) {                                               \
            (void)fprintf(stderr, "cannot assemble log content");           \
        } else {                                                      \
            (void)fprintf(stderr, "%s", (const char *)_content);            \
        }                                                             \
    } while (0)

#define ALLOW_PATH "/devices.allow"
#define ROOT_GAP 4
#define FSTYPE_GAP 2
#define MOUNT_SUBSTR_GAP 2
#define ROOT_SUBSTR_GAP 2

struct PathInfo {
    char* src;
    size_t srcLen;
    char* dst;
    size_t dstLen;
};

struct MountList {
    unsigned int count;
    char list[MAX_MOUNT_NR][PATH_MAX];
};

struct ParsedConfig {
    char rootfs[BUF_SIZE];
    size_t devices[MAX_DEVICE_NR];
    size_t devicesNr;
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
    const struct MountList *files;
    const struct MountList *dirs;
};

void InitParsedConfig(struct ParsedConfig *parsedConfig);

#endif
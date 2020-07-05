/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#ifndef _BASIC_H
#define _BASIC_H

#include <sys/types.h>

#define DEVICE_NAME "davinci"
#define DAVINCI_MANAGER             "davinci_manager"
#define DEVMM_SVM                   "devmm_svm"
#define HISI_HDC                    "hisi_hdc"
#define ASCEND_DRIVER_PATH          "/usr/local/Ascend/driver"
#define ASCEND_ADDONS_PATH          "/usr/local/Ascend/add-ons"
#define DEFAULT_DIR_MODE 0755
#define BUF_SIZE 1024

#define ALLOW_PATH "/devices.allow"
#define ROOT_GAP 4
#define FSTYPE_GAP 2
#define MOUNT_SUBSTR_GAP 2
#define ROOT_SUBSTR_GAP 2

struct CmdArgs {
    char devices[BUF_SIZE];
    char rootfs[BUF_SIZE];
    int  pid;
};

struct PathInfo {
    char* src;
    size_t srcLen;
    char* dst;
    size_t dstLen;
};

#endif
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-cli工具公共宏和结构体定义
*/
#ifndef _BASIC_H
#define _BASIC_H

#include <sys/types.h>
#include <stdint.h>

#define DEVICE_NAME "davinci"
#define DAVINCI_MANAGER             "davinci_manager"
#define DEVMM_SVM                   "devmm_svm"
#define HISI_HDC                    "hisi_hdc"
#define ASCEND_DRIVER_LIB64_PATH    "/usr/local/Ascend/driver/lib64"
#define ASCEND_DRIVER_TOOLS_PATH    "/usr/local/Ascend/driver/tools"
#define ASCEND_DRIVER_INC_PATH      "/usr/local/Ascend/driver/include"
#define ASCEND_ADDONS_PATH          "/usr/local/Ascend/add-ons"
#define ASCEND_DCMI_PATH            "/usr/local/dcmi"
#define ASCEND_NPU_SMI_PATH         "/usr/local/sbin/npu-smi"
#define ASCEND_SLOG_CONF_PATH       "/var/log/npu/conf/slog/slog.conf"
#define DEFAULT_DIR_MODE 0755
#define BUF_SIZE 1024
#define MAX_DEVICE_NR 1024

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

struct ParsedConfig {
    char rootfs[BUF_SIZE];
    unsigned int devices[MAX_DEVICE_NR];
    size_t devicesNr;
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

void InitParsedConfig(struct ParsedConfig *parsedConfig);

#endif
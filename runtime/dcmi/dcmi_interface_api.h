/*
 * Copyright: Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Author: d00583127
 * Date: 2021-03-17 17:46:08
 * @LastEditors: Hu Haoran h00584933
 * @LastEditTime: 2021-07-10 20:13:31
 * Description: DCMI API Reference
 */

/********************************************************************************/

#ifndef __DCMI_INTERFACE_API_H__
#define __DCMI_INTERFACE_API_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef __linux
#define DCMIDLLEXPORT
#else
#define DCMIDLLEXPORT _declspec(dllexport)
#endif

#define MAX_VER_LEN 255  // Maximum length of version string

#define MAX_CARD_NUM 64  // The system supports up to 64 cards

#define MAX_CHIP_NAME_LEN 32  // Maximum length of chip name

#define MAX_LENTH 256  // Maximum length of string

#define DIE_ID_COUNT 5  // Number of die ID characters

#define MAX_CORE_NUM 16  // Maximum AI CPU

#define DCMI_SENSOR_DATA_MAX_LEN 16

#define DCMI_SENSOR_TEMP_LEN 2

#define DCMI_SENSOR_NTC_TEMP_LEN 4

#define DCMI_COMPUTE_GROUP_INFO_RES_NUM 8

#define DCMI_MAX_VDEV_NUM 16  // max number a device can spilts
#define DCMI_MAX_SPEC_RESERVE 8

/*----------------------------------------------*
 * Structure description                        *
 *----------------------------------------------*/
struct dcmi_chip_info {
    unsigned char chip_type[MAX_CHIP_NAME_LEN];
    unsigned char chip_name[MAX_CHIP_NAME_LEN];
    unsigned char chip_ver[MAX_CHIP_NAME_LEN];
    unsigned int aicore_cnt;
};

struct dcmi_pcie_info {
    unsigned int deviceid;
    unsigned int venderid;
    unsigned int subvenderid;
    unsigned int subdeviceid;
    unsigned int bdf_deviceid;
    unsigned int bdf_busid;
    unsigned int bdf_funcid;
};

struct dcmi_pcie_info_all {
    unsigned int venderid;          /* 厂商id */
    unsigned int subvenderid;       /* 厂商子id */
    unsigned int deviceid;          /* 设备id */
    unsigned int subdeviceid;       /* 设备子id */
    int domain;                     /* pcie domain */
    unsigned int bdf_busid;
    unsigned int bdf_deviceid;
    unsigned int bdf_funcid;
    unsigned char reserve[32];      /* the size of dcmi_pcie_info_all is 64 */
};

struct dcmi_board_info {
    unsigned int board_id;
    unsigned int pcb_id;
    unsigned int bom_id;
    unsigned int slot_id;
};

struct dcmi_elabel_info {
    char product_name[MAX_LENTH];
    char model[MAX_LENTH];
    char manufacturer[MAX_LENTH];
    char manufacturer_date[MAX_LENTH];
    char serial_number[MAX_LENTH];
};

struct dcmi_die_id {
    unsigned int soc_die[DIE_ID_COUNT];
};

struct dcmi_aicore_info {
    unsigned int freq;
    unsigned int cur_freq;
};

struct dcmi_aicpu_info {
    unsigned int max_freq;
    unsigned int cur_freq;
    unsigned int aicpu_num;
    unsigned int util_rate[MAX_CORE_NUM];
};

struct dcmi_flash_info {
    unsigned long long flash_id;
    unsigned short device_id;
    unsigned short vendor;
    unsigned int state;
    unsigned long long size;
    unsigned int sector_count;
    unsigned short manufacturer_id;
};

struct dcmi_chip_pcie_err_rate {
    unsigned int reg_deskew_fifo_overflow_intr_status;
    unsigned int reg_symbol_unlock_intr_status;
    unsigned int reg_deskew_unlock_intr_status;
    unsigned int reg_phystatus_timeout_intr_status;
    unsigned int symbol_unlock_counter;
    unsigned int pcs_rx_err_cnt;
    unsigned int phy_lane_err_counter;
    unsigned int pcs_rcv_err_status;
    unsigned int symbol_unlock_err_status;
    unsigned int phy_lane_err_status;
    unsigned int dl_lcrc_err_num;
    unsigned int dl_dcrc_err_num;
};

struct dcmi_ecc_info {
    int enable_flag;
    unsigned int single_bit_error_cnt;
    unsigned int double_bit_error_cnt;
    unsigned int total_single_bit_error_cnt;
    unsigned int total_double_bit_error_cnt;
    unsigned int single_bit_isolated_pages_cnt;
    unsigned int double_bit_isolated_pages_cnt;
};

struct dcmi_hbm_info {
    unsigned long long memory_size;
    unsigned int freq;
    unsigned long long memory_usage;
    int temp;
    unsigned int bandwith_util_rate;
};

struct dcmi_memory_info {
    unsigned long long memory_size;        /* unit:MB */
    unsigned int freq;
    unsigned int utiliza;
};

struct dcmi_get_memory_info_stru {
    unsigned long long memory_size;        /* unit:MB */
    unsigned long long memory_available;   /* free + hugepages_free * hugepagesize */
    unsigned int freq;
    unsigned long hugepagesize;             /* unit:KB */
    unsigned long hugepages_total;
    unsigned long hugepages_free;
    unsigned int utiliza;                  /* ddr memory info usages */
    unsigned char reserve[60];             /* the size of dcmi_memory_info is 96 */
};

struct dcmi_dvpp_ratio {
    int vdec_ratio;
    int vpc_ratio;
    int venc_ratio;
    int jpege_ratio;
    int jpegd_ratio;
};

struct dcmi_capability_group_info {
    unsigned int group_id;
    unsigned int state;
    unsigned int extend_attribute;
    unsigned int aicore_number;
    unsigned int aivector_number;
    unsigned int sdma_number;
    unsigned int aicpu_number;
    unsigned int active_sq_number;
    unsigned int res[DCMI_COMPUTE_GROUP_INFO_RES_NUM];
};

struct dcmi_cgroup_info {
    unsigned long limit_in_bytes;
    unsigned long max_usage_in_bytes;
    unsigned long usage_in_bytes;
};

struct dcmi_llc_perf {
    unsigned int wr_hit_rate;
    unsigned int rd_hit_rate;
    unsigned int throughput;
};

enum dcmi_ip_addr_type {
    DCMI_IPADDR_TYPE_V4 = 0, /** IPv4 */
    DCMI_IPADDR_TYPE_V6 = 1, /** IPv6 */
    DCMI_IPADDR_TYPE_ANY = 2 /** IPv4+IPv6 ("dual-stack") */
};

struct dcmi_ip_addr {
    union {
        unsigned char ip6[16];
        unsigned char ip4[4];
    } u_addr;
    enum dcmi_ip_addr_type ip_type;
};

struct dcmi_vdev_spec_info {
    unsigned char core_num;     /* aicore num for virtual device */
    unsigned char reservesd[DCMI_MAX_SPEC_RESERVE];     /* reserved */
};

struct dcmi_vdev_create_info {
    unsigned int vdev_num;      /* number of vdevice the devid spilt */
    struct dcmi_vdev_spec_info spec[DCMI_MAX_VDEV_NUM];     /* specification of vdevice */
    unsigned int vdevid[DCMI_MAX_VDEV_NUM];     /* id number of vdevice */
};

struct dcmi_sub_vdev_info {
    unsigned int status;       /* whether the vdevice used by container */
    unsigned int vdevid;       /* id number of vdevice */
    unsigned int vfid;
    unsigned long long cid;    /* container id */
    struct dcmi_vdev_spec_info spec; /* specification of vdevice */
};

struct dcmi_vdev_info {
    unsigned int vdev_num;      /* number of vdevice the devid had created */
    struct dcmi_vdev_spec_info spec_unused;     /* resource the devid unallocated */
    struct dcmi_sub_vdev_info vdev[DCMI_MAX_VDEV_NUM];
};

struct dcmi_proc_mem_info {
    int proc_id;
    unsigned long proc_mem_usage;
};

enum dcmi_reset_channel {
    OUTBAND_CHANNEL = 0,
    INBAND_CHANNEL
};

enum dcmi_unit_type {
    NPU_TYPE = 0,
    MCU_TYPE = 1,
    CPU_TYPE = 2,
    INVALID_TYPE = 0xFF
};

enum dcmi_die_type {
    NDIE,
    VDIE
};

enum dcmi_rdfx_detect_result {
    DCMI_RDFX_DETECT_OK = 0,
    DCMI_RDFX_DETECT_SOCK_FAIL = 1,
    DCMI_RDFX_DETECT_RECV_TIMEOUT = 2,
    DCMI_RDFX_DETECT_UNREACH = 3,
    DCMI_RDFX_DETECT_TIME_EXCEEDED = 4,
    DCMI_RDFX_DETECT_FAULT = 5,
    DCMI_RDFX_DETECT_INIT = 6,
    DCMI_RDFX_DETECT_THREAD_ERR = 7,
    DCMI_RDFX_DETECT_IP_SET = 8,
    DCMI_RDFX_DETECT_MAX = 0xFF
};

enum dcmi_port_type {
    DCMI_VNIC_PORT = 0,
    DCMI_ROCE_PORT = 1,
    DCMI_INVALID_PORT
};

enum dcmi_revo_type {
    DCMI_REVOCATION_TYPE_SOC = 0,
    DCMI_REVOCATION_TYPE_MAX
};

enum dcmi_main_cmd {
    DCMI_MAIN_CMD_DVPP = 0,
    DCMI_MAIN_CMD_ISP,
    DCMI_MAIN_CMD_TS_GROUP_NUM,
    DCMI_MAIN_CMD_CAN,
    DCMI_MAIN_CMD_UART,
    DCMI_MAIN_CMD_UPGRADE,
    DCMI_MAIN_CMD_TEMP = 50,
    DCMI_MAIN_CMD_SVM = 51,
    DCMI_MAIN_CMD_VDEV_MNG,
    DCMI_MAIN_CMD_DEVICE_SHARE = 0x8001,
    DCMI_MAIN_CMD_EX_CERT = 0x8003,
    DCMI_MAIN_CMD_MAX
};

/* DSMI sub vdev mng CMD def */
typedef enum {
    DCMI_VMNG_SUB_CMD_GET_VDEV_RESOURCE,
    DCMI_VMNG_SUB_CMD_GET_TOTAL_RESOURCE,
    DCMI_VMNG_SUB_CMD_GET_FREE_RESOURCE,
    DCMI_VMNG_SUB_CMD_MAX,
} DCMI_VDEV_MNG_SUB_CMD;

enum dcmi_component_type {
    DCMI_COMPONENT_TYPE_NVE,
    DCMI_COMPONENT_TYPE_XLOADER,
    DCMI_COMPONENT_TYPE_M3FW,
    DCMI_COMPONENT_TYPE_UEFI,
    DCMI_COMPONENT_TYPE_TEE,
    DCMI_COMPONENT_TYPE_KERNEL,
    DCMI_COMPONENT_TYPE_DTB,
    DCMI_COMPONENT_TYPE_ROOTFS,
    DCMI_COMPONENT_TYPE_IMU,
    DCMI_COMPONENT_TYPE_IMP,
    DCMI_COMPONENT_TYPE_AICPU,
    DCMI_COMPONENT_TYPE_HBOOT1_A,
    DCMI_COMPONENT_TYPE_HBOOT1_B,
    DCMI_COMPONENT_TYPE_HBOOT2,
    DCMI_COMPONENT_TYPE_DDR,
    DCMI_COMPONENT_TYPE_LP,
    DCMI_COMPONENT_TYPE_HSM,
    DCMI_COMPONENT_TYPE_SAFETY_ISLAND,
    DCMI_COMPONENT_TYPE_HILINK,
    DCMI_COMPONENT_TYPE_RAWDATA,
    DCMI_COMPONENT_TYPE_SYSDRV,
    DCMI_COMPONENT_TYPE_ADSAPP,
    DCMI_COMPONENT_TYPE_COMISOLATOR,
    DCMI_COMPONENT_TYPE_CLUSTER,
    DCMI_COMPONENT_TYPE_CUSTOMIZED,
    DCMI_COMPONENT_TYPE_SYS_BASE_CONFIG,
    DCMI_COMPONENT_TYPE_MAX,
    DCMI_UPGRADE_AND_RESET_ALL_COMPONENT = 0xFFFFFFF7,
    DCMI_UPGRADE_ALL_IMAGE_COMPONENT = 0xFFFFFFFD,
    DCMI_UPGRADE_ALL_FIRMWARE_COMPONENT = 0xFFFFFFFE,
    DCMI_UPGRADE_ALL_COMPONENT = 0xFFFFFFFF
};

enum dcmi_upgrade_state {
    DCMI_UPGRADE_IDLE = 0,
    DCMI_UPGRADE_UPGRADING = 1,
    DCMI_UPGRADE_NOT_SUPPORT = 2,
    DCMI_UPGRADE_UPGRADE_FAIL = 3,
    DCMI_UPGRADE_STATE_NONE
};

enum dcmi_upgrade_type {
    MCU_UPGRADE_START = 1,
    MCU_UPGRADE_VALIDETE = 3,
    MCU_UPGRADE_NONE
};

enum dcmi_boot_status {
    DCMI_BOOT_STATUS_UNINIT = 0,
    DCMI_BOOT_STATUS_BIOS,
    DCMI_BOOT_STATUS_OS,
    DCMI_BOOT_STATUS_FINISH
};

enum dcmi_device_type {
    DCMI_DEVICE_TYPE_DDR,
    DCMI_DEVICE_TYPE_SRAM,
    DCMI_DEVICE_TYPE_HBM,
    DCMI_DEVICE_TYPE_NPU,
    DCMI_DEVICE_TYPE_NONE = 0xff
};

enum dcmi_freq_type {
    DCMI_FREQ_DDR = 1,
    DCMI_FREQ_CTRLCPU = 2,
    DCMI_FREQ_HBM = 6,
    DCMI_FREQ_AICORE_CURRENT_ = 7,
    DCMI_FREQ_AICORE_MAX = 9,
    DCMI_FREQ_VECTORCORE_CURRENT = 12
};

#define    DCMI_UTILIZATION_RATE_DDR             1
#define    DCMI_UTILIZATION_RATE_AICORE          2
#define    DCMI_UTILIZATION_RATE_AICPU           3
#define    DCMI_UTILIZATION_RATE_CTRLCPU         4
#define    DCMI_UTILIZATION_RATE_DDR_BANDWIDTH   5
#define    DCMI_UTILIZATION_RATE_HBM             6
#define    DCMI_UTILIZATION_RATE_HBM_BANDWIDTH   10
#define    DCMI_UTILIZATION_RATE_VECTORCORE      12

enum dcmi_manager_sensor_id {
    DCMI_CLUSTER_TEMP_ID = 0,
    DCMI_PERI_TEMP_ID = 1,
    DCMI_AICORE0_TEMP_ID,
    DCMI_AICORE1_TEMP_ID,
    DCMI_AICORE_LIMIT_ID,
    DCMI_AICORE_TOTAL_PER_ID,
    DCMI_AICORE_ELIM_PER_ID,
    DCMI_AICORE_BASE_FREQ_ID,
    DCMI_NPU_DDR_FREQ_ID,
    DCMI_THERMAL_THRESHOLD_ID,
    DCMI_NTC_TEMP_ID,
    DCMI_SOC_TEMP_ID,
    DCMI_FP_TEMP_ID,
    DCMI_N_DIE_TEMP_ID,
    DCMI_HBM_TEMP_ID,
    DCMI_SENSOR_INVALID_ID = 255
};

union dcmi_sensor_info {
    unsigned char uchar;
    unsigned short ushort;
    unsigned int uint;
    signed int iint;
    signed char temp[DCMI_SENSOR_TEMP_LEN];
    signed int ntc_tmp[DCMI_SENSOR_NTC_TEMP_LEN];
    unsigned int data[DCMI_SENSOR_DATA_MAX_LEN];
};

/*----------------------------------------------*
 *         dcmi_get_fault_event                 *
 *----------------------------------------------*/
#define DCMI_EVENT_FILTER_FLAG_EVENT_ID (1UL << 0)
#define DCMI_EVENT_FILTER_FLAG_SERVERITY (1UL << 1)
#define DCMI_EVENT_FILTER_FLAG_NODE_TYPE (1UL << 2)

#define DCMI_MAX_EVENT_NAME_LENGTH 256
#define DCMI_MAX_EVENT_DATA_LENGTH 32
#define DCMI_MAX_EVENT_RESV_LENGTH 32

struct dcmi_event_filter {
    unsigned long long filter_flag;
    unsigned int event_id;
    unsigned char severity;
    unsigned char node_type;
    unsigned char resv[DCMI_MAX_EVENT_RESV_LENGTH]; /**< reserve 32byte */
};

struct dcmi_dms_fault_event {
    unsigned int event_id;
    unsigned short deviceid;
    unsigned char node_type;
    unsigned char node_id;
    unsigned char sub_node_type;
    unsigned char sub_node_id;
    unsigned char severity;
    unsigned char assertion;
    int event_serial_num;
    int notify_serial_num;
    unsigned long long alarm_raised_time;
    char event_name[DCMI_MAX_EVENT_NAME_LENGTH];
    char additional_info[DCMI_MAX_EVENT_DATA_LENGTH];
    unsigned char resv[DCMI_MAX_EVENT_RESV_LENGTH]; /**< reserve 32byte */
};

enum dcmi_event_type {
    DCMI_DMS_FAULT_EVENT = 0,
    DCMI_EVENT_TYPE_MAX
};

struct dcmi_event {
    enum dcmi_event_type type;
    union {
        struct dcmi_dms_fault_event dms_event;
    } event_t;
};

#define DCMI_VDEV_RES_NAME_LEN 16
#define DCMI_VDEV_FOR_RESERVE 32
struct dcmi_base_resource {
    unsigned long long token;
    unsigned long long token_max;
    unsigned long long task_timeout;
    unsigned int vfg_id;
    unsigned char vip_mode;
    unsigned char reserved[DCMI_VDEV_FOR_RESERVE - 1];  /* bytes aligned */
};

/* total types of computing resource */
struct dcmi_computing_resource {
    /* accelator resource */
    float aic;
    float aiv;
    unsigned short dsa;
    unsigned short rtsq;
    unsigned short acsq;
    unsigned short cdqm;
    unsigned short c_core;
    unsigned short ffts;
    unsigned short sdma;
    unsigned short pcie_dma;

    /* memory resource, MB as unit */
    unsigned long long memory_size;

    /* id resource */
    unsigned int event_id;
    unsigned int notify_id;
    unsigned int stream_id;
    unsigned int model_id;

    /* cpu resource */
    unsigned short topic_schedule_aicpu;
    unsigned short host_ctrl_cpu;
    unsigned short host_aicpu;
    unsigned short device_aicpu;
    unsigned short topic_ctrl_cpu_slot;

    unsigned char reserved[DCMI_VDEV_FOR_RESERVE];
};

/* configurable computing resource */
struct dcmi_computing_configurable {
    /* memory resource, MB as unit */
    unsigned long long memory_size;

    /* accelator resource */
    float aic;
    float aiv;
    unsigned short dsa;
    unsigned short rtsq;
    unsigned short cdqm;

    /* cpu resource */
    unsigned short topic_schedule_aicpu;
    unsigned short host_ctrl_cpu;
    unsigned short host_aicpu;
    unsigned short device_aicpu;

    unsigned char reserved[DCMI_VDEV_FOR_RESERVE];
};
struct dcmi_media_resource {
    /* dvpp resource */
    float jpegd;
    float jpege;
    float vpc;
    float vdec;
    float pngd;
    float venc;
    unsigned char reserved[DCMI_VDEV_FOR_RESERVE];
};

struct dcmi_create_vdev_in {
    char name[DCMI_VDEV_RES_NAME_LEN];
    struct dcmi_base_resource base;
    struct dcmi_computing_configurable computing;
    struct dcmi_media_resource media;
};

struct dcmi_create_vdev_out {
    unsigned int vdev_id;
    unsigned int pcie_bus;
    unsigned int pcie_device;
    unsigned int pcie_func;
    unsigned int vfg_id;
    unsigned char reserved[DCMI_VDEV_FOR_RESERVE];
};

struct dcmi_vdev_query_info {
    char name[DCMI_VDEV_RES_NAME_LEN];
    unsigned int status;
    unsigned int is_container_used;
    unsigned int vfid;
    unsigned int vfg_id;
    unsigned long long container_id;
    struct dcmi_base_resource base;
    struct dcmi_computing_resource computing;
    struct dcmi_media_resource media;
};

/* for single search */
struct dcmi_vdev_query_stru {
    unsigned int vdev_id;
    struct dcmi_vdev_query_info query_info;
};

struct dcmi_soc_free_resource {
    unsigned int vfg_num;
    unsigned int vfg_bitmap;
    struct dcmi_base_resource base;
    struct dcmi_computing_resource computing;
    struct dcmi_media_resource media;
};

struct dcmi_soc_total_resource {
    unsigned int vdev_num;
    unsigned int vdev_id[32];
    unsigned int vfg_num;
    unsigned int vfg_bitmap;
    struct dcmi_base_resource base;
    struct dcmi_computing_resource computing;
    struct dcmi_media_resource media;
};

/*----------------------------------------------*
 * Error code description                       *
 *----------------------------------------------*/

#define DCMI_OK 0
#define DCMI_ERROR_CODE_BASE -8000
#define DCMI_ERR_CODE_INVALID_PARAMETER             (DCMI_ERROR_CODE_BASE - 1)
#define DCMI_ERR_CODE_OPER_NOT_PERMITTED            (DCMI_ERROR_CODE_BASE - 2)
#define DCMI_ERR_CODE_MEM_OPERATE_FAIL              (DCMI_ERROR_CODE_BASE - 3)
#define DCMI_ERR_CODE_SECURE_FUN_FAIL               (DCMI_ERROR_CODE_BASE - 4)
#define DCMI_ERR_CODE_INNER_ERR                     (DCMI_ERROR_CODE_BASE - 5)
#define DCMI_ERR_CODE_TIME_OUT                      (DCMI_ERROR_CODE_BASE - 6)
#define DCMI_ERR_CODE_INVALID_DEVICE_ID             (DCMI_ERROR_CODE_BASE - 7)
#define DCMI_ERR_CODE_DEVICE_NOT_EXIST              (DCMI_ERROR_CODE_BASE - 8)
#define DCMI_ERR_CODE_IOCTL_FAIL                    (DCMI_ERROR_CODE_BASE - 9)
#define DCMI_ERR_CODE_SEND_MSG_FAIL                 (DCMI_ERROR_CODE_BASE - 10)
#define DCMI_ERR_CODE_RECV_MSG_FAIL                 (DCMI_ERROR_CODE_BASE - 11)
#define DCMI_ERR_CODE_NOT_REDAY                     (DCMI_ERROR_CODE_BASE - 12)
#define DCMI_ERR_CODE_NOT_SUPPORT_IN_CONTAINER      (DCMI_ERROR_CODE_BASE - 13)
#define DCMI_ERR_CODE_FILE_OPERATE_FAIL             (DCMI_ERROR_CODE_BASE - 14)
#define DCMI_ERR_CODE_RESET_FAIL                    (DCMI_ERROR_CODE_BASE - 15)
#define DCMI_ERR_CODE_ABORT_OPERATE                 (DCMI_ERROR_CODE_BASE - 16)
#define DCMI_ERR_CODE_IS_UPGRADING                  (DCMI_ERROR_CODE_BASE - 17)
#define DCMI_ERR_CODE_RESOURCE_OCCUPIED             (DCMI_ERROR_CODE_BASE - 20)
#define DCMI_ERR_CODE_PARTITION_NOT_RIGHT           (DCMI_ERROR_CODE_BASE - 22)
#define DCMI_ERR_CODE_NOT_SUPPORT                   (DCMI_ERROR_CODE_BASE - 255)

#define DCMI_VERSION_1
#define DCMI_VERSION_2

#if defined DCMI_VERSION_2

DCMIDLLEXPORT int dcmi_init(void);

DCMIDLLEXPORT int dcmi_get_dcmi_version(char *dcmi_ver, unsigned int len);

DCMIDLLEXPORT int dcmi_get_driver_version(char *driver_ver, unsigned int len);

DCMIDLLEXPORT int dcmi_get_card_list(int *card_num, int *card_list, int list_len);

DCMIDLLEXPORT int dcmi_get_device_num_in_card(int card_id, int *device_num);

DCMIDLLEXPORT int dcmi_get_device_id_in_card(int card_id, int *device_id_max, int *mcu_id, int *cpu_id);

DCMIDLLEXPORT int dcmi_get_device_type(int card_id, int device_id, enum dcmi_unit_type *device_type);

DCMIDLLEXPORT int dcmi_get_device_chip_info(int card_id, int device_id, struct dcmi_chip_info *chip_info);

DCMIDLLEXPORT int dcmi_get_device_pcie_info(int card_id, int device_id, struct dcmi_pcie_info *pcie_info);

DCMIDLLEXPORT int dcmi_get_device_pcie_info_v2(int card_id, int device_id, struct dcmi_pcie_info_all *pcie_info);

DCMIDLLEXPORT int dcmi_get_device_board_info(int card_id, int device_id, struct dcmi_board_info *board_info);

DCMIDLLEXPORT int dcmi_get_device_elabel_info(int card_id, int device_id, struct dcmi_elabel_info *elabel_info);

DCMIDLLEXPORT int dcmi_get_device_power_info(int card_id, int device_id, int *power);

DCMIDLLEXPORT int dcmi_set_card_customized_info(int card_id, char *info, int len);

DCMIDLLEXPORT int dcmi_get_card_customized_info(int card_id, char *info, int len);

DCMIDLLEXPORT int dcmi_set_device_clear_pcie_error(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_get_device_pcie_error_cnt(
    int card_id, int device_id, struct dcmi_chip_pcie_err_rate *pcie_err_code_info);

DCMIDLLEXPORT int dcmi_get_device_die_v2(
    int card_id, int device_id, enum dcmi_die_type input_type, struct dcmi_die_id *die_id);

DCMIDLLEXPORT int dcmi_get_device_health(int card_id, int device_id, unsigned int *health);

DCMIDLLEXPORT int dcmi_get_device_errorcode_v2(
    int card_id, int device_id, int *error_count, unsigned int *error_code_list, unsigned int list_len);

DCMIDLLEXPORT int dcmi_get_device_errorcode_string(
    int card_id, int device_id, unsigned int error_code, unsigned char *error_info, int buf_size);

DCMIDLLEXPORT int dcmi_get_device_flash_count(int card_id, int device_id, unsigned int *flash_count);

DCMIDLLEXPORT int dcmi_get_device_flash_info_v2(
    int card_id, int device_id, unsigned int flash_index, struct dcmi_flash_info *flash_info);

DCMIDLLEXPORT int dcmi_get_device_aicore_info(int card_id, int device_id, struct dcmi_aicore_info *aicore_info);

DCMIDLLEXPORT int dcmi_get_device_aicpu_info(int card_id, int device_id, struct dcmi_aicpu_info *aicpu_info);

DCMIDLLEXPORT int dcmi_get_device_boot_status(int card_id, int device_id, enum dcmi_boot_status *boot_status);

DCMIDLLEXPORT int dcmi_set_device_pre_reset(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_set_device_rescan(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_set_device_reset(int card_id, int device_id, enum dcmi_reset_channel channel_type);

DCMIDLLEXPORT int dcmi_get_mcu_upgrade_status(int card_id, int *status, int *progress);

DCMIDLLEXPORT int dcmi_get_mcu_version(int card_id, char *version, int len);

DCMIDLLEXPORT int dcmi_set_mcu_upgrade_stage(int card_id, enum dcmi_upgrade_type input_type);

DCMIDLLEXPORT int dcmi_set_mcu_upgrade_file(int card_id, const char *file);

DCMIDLLEXPORT int dcmi_get_device_system_time(int card_id, int device_id, unsigned int *time);

DCMIDLLEXPORT int dcmi_get_device_temperature(int card_id, int device_id, int *temperature);

DCMIDLLEXPORT int dcmi_get_device_voltage(int card_id, int device_id, unsigned int *voltage);

DCMIDLLEXPORT int dcmi_get_device_p2p_enable(int card_id, int device_id, int *enable_flag);

DCMIDLLEXPORT int dcmi_get_device_ecc_info(
    int card_id, int device_id, enum dcmi_device_type input_type, struct dcmi_ecc_info *device_ecc_info);

DCMIDLLEXPORT int dcmi_set_device_clear_ecc_statistics_info(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_get_device_frequency(
    int card_id, int device_id, enum dcmi_freq_type input_type, unsigned int *frequency);

DCMIDLLEXPORT int dcmi_get_device_hbm_info(int card_id, int device_id, struct dcmi_hbm_info *hbm_info);

DCMIDLLEXPORT int dcmi_get_device_memory_info_v2(int card_id, int device_id, struct dcmi_memory_info *memory_info);

DCMIDLLEXPORT int dcmi_get_device_memory_info_v3(int card_id, int device_id,
    struct dcmi_get_memory_info_stru *memory_info);

DCMIDLLEXPORT int dcmi_get_device_utilization_rate(
    int card_id, int device_id, int input_type, unsigned int *utilization_rate);

DCMIDLLEXPORT int dcmi_get_device_sensor_info(
    int card_id, int device_id, enum dcmi_manager_sensor_id sensor_id, union dcmi_sensor_info *sensor_info);

DCMIDLLEXPORT int dcmi_set_container_service_enable(void);

DCMIDLLEXPORT int dcmi_get_device_board_id(int card_id, int device_id, unsigned int *board_id);

DCMIDLLEXPORT int dcmi_get_device_component_count(int card_id, int device_id, unsigned int *component_count);

DCMIDLLEXPORT int dcmi_get_device_component_list(
    int card_id, int device_id, enum dcmi_component_type *component_table, unsigned int component_count);

DCMIDLLEXPORT int dcmi_get_device_component_static_version(
    int card_id, int device_id, enum dcmi_component_type component_type, unsigned char *version_str, unsigned int len);

DCMIDLLEXPORT int dcmi_get_device_cgroup_info(int card_id, int device_id, struct dcmi_cgroup_info *cg_info);

DCMIDLLEXPORT int dcmi_get_device_llc_perf_para(int card_id, int device_id, struct dcmi_llc_perf *perf_para);

DCMIDLLEXPORT int dcmi_set_device_info(int card_id, int device_id, enum dcmi_main_cmd main_cmd, unsigned int sub_cmd,
    const void *buf, unsigned int buf_size);

DCMIDLLEXPORT int dcmi_get_device_info(
    int card_id, int device_id, enum dcmi_main_cmd main_cmd, unsigned int sub_cmd, void *buf, unsigned int *size);

DCMIDLLEXPORT int dcmi_set_device_sec_revocation(
    int card_id, int device_id, enum dcmi_revo_type input_type, const unsigned char *file_data, unsigned int file_size);

DCMIDLLEXPORT int dcmi_get_device_mac_count(int card_id, int device_id, int *count);

DCMIDLLEXPORT int dcmi_set_device_mac(int card_id, int device_id, int mac_id, const char *mac_addr, unsigned int len);

DCMIDLLEXPORT int dcmi_get_device_mac(int card_id, int device_id, int mac_id, char *mac_addr, unsigned int len);

DCMIDLLEXPORT int dcmi_get_device_gateway(
    int card_id, int device_id, enum dcmi_port_type input_type, int port_id, struct dcmi_ip_addr *gateway);

DCMIDLLEXPORT int dcmi_set_device_gateway(
    int card_id, int device_id, enum dcmi_port_type input_type, int port_id, struct dcmi_ip_addr *gateway);

DCMIDLLEXPORT int dcmi_set_device_ip(int card_id, int device_id, enum dcmi_port_type input_type, int port_id,
    struct dcmi_ip_addr *ip, struct dcmi_ip_addr *mask);

DCMIDLLEXPORT int dcmi_get_device_ip(int card_id, int device_id, enum dcmi_port_type input_type, int port_id,
    struct dcmi_ip_addr *ip, struct dcmi_ip_addr *mask);

DCMIDLLEXPORT int dcmi_get_device_network_health(int card_id, int device_id, enum dcmi_rdfx_detect_result *result);

DCMIDLLEXPORT int dcmi_get_device_fan_count(int card_id, int device_id, int *count);

DCMIDLLEXPORT int dcmi_get_device_fan_speed(int card_id, int device_id, int fan_id, int *speed);

DCMIDLLEXPORT int dcmi_get_device_logic_id(int *device_logic_id, int card_id, int device_id);

DCMIDLLEXPORT int dcmi_get_card_elabel_v2(int card_id, struct dcmi_elabel_info *elabel_info);

DCMIDLLEXPORT int dcmi_mcu_get_chip_temperature(int card_id, char *data_info, int buf_size, int *data_len);

DCMIDLLEXPORT int dcmi_get_device_ssh_enable(int card_id, int device_id, int *enable_flag);

DCMIDLLEXPORT int dcmi_set_device_share_enable(int card_id, int device_id, int enable_flag);

DCMIDLLEXPORT int dcmi_get_device_share_enable(int card_id, int device_id, int *enable_flag);

DCMIDLLEXPORT int dcmi_get_card_board_info(int card_id, struct dcmi_board_info *board_info);

DCMIDLLEXPORT int dcmi_get_card_pcie_info(int card_id, char *pcie_info, int pcie_info_len);

DCMIDLLEXPORT int dcmi_get_card_pcie_slot(int card_id, int *pcie_slot);

DCMIDLLEXPORT int dcmi_get_fault_device_num_in_card(int card_id, int *device_num);

DCMIDLLEXPORT int dcmi_mcu_check_i2c(int card_id, int *health_status, int buf_size);

DCMIDLLEXPORT int dcmi_set_device_user_config(
    int card_id, int device_id, const char *config_name, unsigned int buf_size, char *buf);

DCMIDLLEXPORT int dcmi_mcu_collect_log(int card_id, int log_type);

DCMIDLLEXPORT int dcmi_get_device_chip_slot(int card_id, int device_id, int *chip_pos_id);

DCMIDLLEXPORT int dcmi_get_product_type(int card_id, int device_id, char *product_type_str, int buf_size);

DCMIDLLEXPORT int dcmi_get_device_outband_channel_state(int card_id, int device_id, int *channel_state);

DCMIDLLEXPORT int dcmi_get_device_aicpu_count_info(int card_id, int device_id, unsigned char *count_info);

DCMIDLLEXPORT int dcmi_create_vdevice(int card_id, int device_id, int vdev_id, const char *template_name,
    struct dcmi_create_vdev_out *out);

DCMIDLLEXPORT int dcmi_set_destroy_vdevice(int card_id, int device_id, unsigned int vdevid);

DCMIDLLEXPORT int dcmi_get_board_id(int card_id, int device_id, int *board_id);

DCMIDLLEXPORT int dcmi_get_first_power_on_date(int card_id, unsigned int *first_power_on_date);

DCMIDLLEXPORT int dcmi_get_fault_event(int card_id, int device_id, int timeout, struct dcmi_event_filter filter,
    struct dcmi_event *event);

DCMIDLLEXPORT int dcmi_get_device_resource_info(int card_id, int device_id,
    struct dcmi_proc_mem_info *proc_info, int *proc_num);

DCMIDLLEXPORT int dcmi_get_device_dvpp_ratio_info(int card_id, int device_id, struct dcmi_dvpp_ratio *usage);

#endif

#if defined DCMI_VERSION_1
/* The following interfaces are V1 version interfaces. In order to ensure the compatibility is temporarily reserved,
 * the later version will be deleted. Please switch to the V2 version interface as soon as possible */

struct dcmi_tag_pcie_idinfo {
    unsigned int deviceid;
    unsigned int venderid;
    unsigned int subvenderid;
    unsigned int subdeviceid;
    unsigned int bdf_deviceid;
    unsigned int bdf_busid;
    unsigned int bdf_funcid;
};

struct dcmi_board_info_stru {
    unsigned int board_id;
    unsigned int pcb_id;
    unsigned int bom_id;
    unsigned int slot_id;
};

typedef struct dcmi_elabel_info_stru {
    char product_name[MAX_LENTH];
    char model[MAX_LENTH];
    char manufacturer[MAX_LENTH];
    char serial_number[MAX_LENTH];
}DCMI_ELABEL_INFO_STRU, *PDCMI_ELABEL_INFO_STRU;

struct dcmi_chip_pcie_err_rate_stru {
    unsigned int reg_deskew_fifo_overflow_intr_status;
    unsigned int reg_symbol_unlock_intr_status;
    unsigned int reg_deskew_unlock_intr_status;
    unsigned int reg_phystatus_timeout_intr_status;
    unsigned int symbol_unlock_counter;
    unsigned int pcs_rx_err_cnt;
    unsigned int phy_lane_err_counter;
    unsigned int pcs_rcv_err_status;
    unsigned int symbol_unlock_err_status;
    unsigned int phy_lane_err_status;
    unsigned int dl_lcrc_err_num;
    unsigned int dl_dcrc_err_num;
};

struct dcmi_soc_die_stru {
    unsigned int soc_die[5];
};

struct dcmi_flash_info_stru {
    unsigned long long flash_id;     /* combined device & manufacturer code */
    unsigned short device_id;        /* device id */
    unsigned short vendor;           /* the primary vendor id */
    unsigned int state;              /* flash health */
    unsigned long long size;         /* total size in bytes */
    unsigned int sector_count;       /* number of erase units */
    unsigned short manufacturer_id;  /* manufacturer id */
};

struct dcmi_ecc_info_stru {
    int enable_flag;
    unsigned int single_bit_error_count;
    unsigned int double_bit_error_count;
};

struct dcmi_memory_info_stru {
    unsigned long long memory_size;       // 单位 MB
    unsigned int freq;
    unsigned int utiliza;
};

#ifndef __DSMI_COMMON_INTERFACE_H__

struct dsmi_soc_die_stru {
    unsigned int soc_die[5];
};

struct dsmi_board_info_stru {
    unsigned int board_id;
    unsigned int pcb_id;
    unsigned int bom_id;
    unsigned int slot_id;
};

struct tag_pcie_idinfo {
    unsigned int deviceid;
    unsigned int venderid;
    unsigned int subvenderid;
    unsigned int subdeviceid;
    unsigned int bdf_deviceid;
    unsigned int bdf_busid;
    unsigned int bdf_funcid;
};

struct dm_flash_info_stru {
    unsigned long flash_id;         /* combined device & manufacturer code */
    unsigned short device_id;       /* device id */
    unsigned short vendor;          /* the primary vendor id */
    unsigned int state;             /* flash health, 0x8:normal,0x10:abnormal */
    unsigned long size;             /* total size in bytes */
    unsigned int sector_count;      /* number of erase units */
    unsigned short manufacturer_id; /* manufacturer id */
};

struct dsmi_aicore_info_stru {
    unsigned int freq;              /* normal freq */
    unsigned int curfreq;           /* current freq */
};

typedef struct dsmi_aicpu_info_stru {
    unsigned int maxFreq;
    unsigned int curFreq;
    unsigned int aicpuNum;
    unsigned int utilRate[16];
} DSMI_AICPU_INFO;

struct dsmi_ecc_info_stru {
    int enable_flag;
    unsigned int single_bit_error_count;
    unsigned int double_bit_error_count;
};

struct dsmi_ecc_pages_stru {
    unsigned int corrected_ecc_errors_aggregate_total;   // 生命周期内所有可纠正ecc错误统计
    unsigned int uncorrected_ecc_errors_aggregate_total; // 生命周期内所有不可纠正ecc错误统计
    unsigned int isolated_pages_single_bit_error;        // 单bit错误隔离内存页数量
    unsigned int isolated_pages_double_bit_error;        // 多bit错误隔离内存页数量
};

struct dsmi_hbm_info_stru {
    unsigned long long memory_size;      /**< HBM total size, KB */
    unsigned int freq;                   /**< HBM freq, MHZ */
    unsigned long long memory_usage;     /**< HBM memory_usage, KB */
    int temp;                            /**< HBM temperature */
    unsigned int bandwith_util_rate;
};

struct dsmi_memory_info_stru {
    unsigned long long memory_size;
    unsigned int freq;
    unsigned int utiliza;
};

typedef union tag_sensor_info {
    unsigned char uchar;
    unsigned short ushort;
    unsigned int uint;
    signed int iint;
    signed char temp[2];       /* <  2 temp size */
    signed int ntc_tmp[4];     /* <  4 ntc_tmp size */
    unsigned int data[16];
} TAG_SENSOR_INFO;

struct dsmi_computing_power_info {
    unsigned int data1;
    unsigned int reserve[3];
};
#endif /* __DSMI_COMMON_INTERFACE_H__ */

DCMIDLLEXPORT int dcmi_get_card_num_list(int *card_num, int *card_list, int list_len);

DCMIDLLEXPORT int dcmi_get_pcie_info(int card_id, int device_id, struct dcmi_tag_pcie_idinfo *pcie_idinfo);

DCMIDLLEXPORT int dcmi_get_board_info(int card_id, int device_id, struct dcmi_board_info_stru *board_info);

DCMIDLLEXPORT int dcmi_get_card_elabel(int card_id, struct dcmi_elabel_info_stru *elabel_info);

DCMIDLLEXPORT int dcmi_mcu_set_license_info(int card_id, char *license, int len);

DCMIDLLEXPORT int dcmi_mcu_get_license_info(int card_id, char *data_info, int *len);

DCMIDLLEXPORT int dcmi_get_customized_info_api(int card_id, char *data_info, int *len);

DCMIDLLEXPORT int dcmi_set_customized_info_api(int card_id, const char *data_info, int len);

DCMIDLLEXPORT int dcmi_clear_pcie_error_cnt(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_get_pcie_error_cnt(
    int card_id, int device_id, struct dcmi_chip_pcie_err_rate_stru *pcie_err_code_info);

DCMIDLLEXPORT int dcmi_get_device_die(int card_id, int device_id, struct dcmi_soc_die_stru *device_die);

DCMIDLLEXPORT int dcmi_get_device_ndie(int card_id, int device_id, struct dsmi_soc_die_stru *device_ndie);

DCMIDLLEXPORT int dcmi_get_device_errorcode(
    int card_id, int device_id, int *error_count, unsigned int *error_code, int *error_width);

DCMIDLLEXPORT int dcmi_get_device_errorinfo(
    int card_id, int device_id, int errorcode, unsigned char *errorinfo, int buf_size);

DCMIDLLEXPORT int dcmi_get_device_flash_info(
    int card_id, int device_id, unsigned int flash_index, struct dcmi_flash_info_stru *flash_info);

DCMIDLLEXPORT int dcmi_get_aicore_info(int card_id, int device_id, struct dsmi_aicore_info_stru *aicore_info);

DCMIDLLEXPORT int dcmi_get_aicpu_info(int card_id, int device_id, struct dsmi_aicpu_info_stru *aicpu_info);

DCMIDLLEXPORT int dcmi_pre_reset_soc(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_rescan_soc(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_reset_device(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_mcu_get_upgrade_statues(int card_id, int *status, int *progress);

DCMIDLLEXPORT int dcmi_mcu_get_upgrade_status(int card_id, int *status, int *progress);

DCMIDLLEXPORT int dcmi_mcu_get_version(int card_id, char *version_str, int max_version_len, int *len);

DCMIDLLEXPORT int dcmi_mcu_upgrade_control(int card_id, int upgrade_type);

DCMIDLLEXPORT int dcmi_mcu_upgrade_transfile(int card_id, const char *file);

DCMIDLLEXPORT int dcmi_get_p2p_enable(int card_id, int device_id, int *enable_flag);

DCMIDLLEXPORT int dcmi_get_ecc_info(
    int card_id, int device_id, int device_type, struct dsmi_ecc_info_stru *device_ecc_info);

DCMIDLLEXPORT int dcmi_get_hbm_info(int card_id, int device_id, struct dsmi_hbm_info_stru *device_hbm_info);

DCMIDLLEXPORT int dcmi_get_memory_info(int card_id, int device_id, struct dcmi_memory_info_stru *device_memory_info);

DCMIDLLEXPORT int dcmi_get_soc_sensor_info(
    int card_id, int device_id, int sensor_id, union tag_sensor_info *sensor_info);

DCMIDLLEXPORT int dcmi_config_ecc_enable(int card_id, int device_id, int enable_flag);

DCMIDLLEXPORT int dcmi_get_version(int card_id, int device_id, char *verison_str, unsigned int version_len, int *len);

DCMIDLLEXPORT int dcmi_mcu_get_board_info(int card_id, struct dcmi_board_info *board_info);

DCMIDLLEXPORT int dcmi_mcu_get_power_info(int card_id, int *power);

DCMIDLLEXPORT int dcmi_get_computing_power_info(
    int card_id, int device_id, int type, struct dsmi_computing_power_info *computing_power);

DCMIDLLEXPORT int dcmi_set_device_ecc_enable(
    int card_id, int device_id, enum dcmi_device_type device_type, int enable_flag);

DCMIDLLEXPORT int dcmi_set_user_config(
    int card_id, int device_id, const char *config_name, unsigned int buf_size, unsigned char *buf);

DCMIDLLEXPORT int dcmi_get_user_config(
    int card_id, int device_id, const char *config_name, unsigned int buf_size, unsigned char *buf);

DCMIDLLEXPORT int dcmi_clear_device_user_config(int card_id, int device_id, const char *config_name);

DCMIDLLEXPORT int dcmi_reset_device_inband(int card_id, int device_id);

DCMIDLLEXPORT int dcmi_get_nve_level(int card_id, int device_id, int *nve_level);

DCMIDLLEXPORT int dcmi_set_nve_level(int card_id, int device_id, int level);

DCMIDLLEXPORT int dcmi_get_system_time(int card_id, int device_id, unsigned int *time);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DCMI_INTERFACE_API_H__ */

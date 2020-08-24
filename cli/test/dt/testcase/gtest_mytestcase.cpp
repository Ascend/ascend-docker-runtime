/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: 测试集
*/
#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include <sys/mount.h>

using namespace std;
using namespace testing;

#define DAVINCI_MANAGER_PATH        "/dev/davinci_manager"
#define BUF_SIZE 1024
#define MAX_DEVICE_NR 1024
typedef char *(*ParseFileLine)(char *, const char *);
extern "C" int IsStrEqual(const char *s1, const char *s2);
extern "C" int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
extern "C"  int snprintf_s(char *strDest, size_t destMax, size_t count, const char *format, ...);
extern "C" int open(const char *path, int flags);
extern "C" int close(int fd);
extern "C" int stat(const char *file_name, struct stat *buf);
extern "C" int mount(const char *source, const char *target,
                     const char *filesystemtype, unsigned long mountflags, const void *data);
extern "C" int Mount(const char *src, const char *dst);
extern "C" int MkDir(const char *dir, int mode);
extern "C" int rmdir(const char *pathname);
extern "C" int EnterNsByFd(int fd, int nsType);
extern "C" int StrHasPrefix(const char *str, const char *prefix);
extern "C" int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
extern "C" int GetSelfNsPath(const char *nsType, char *buf, size_t bufSize);
extern "C" int EnterNsByPath(const char *path, int nsType);
extern "C" int MountDevice(const char *rootfs, const char *deviceName);
extern "C" int DoDeviceMounting(const char *rootfs, const unsigned int ids[], size_t idsNr);
extern "C" int CheckDirExists(char *dir, int len);
extern "C" int GetParentPathStr(const char *path, char *parent, size_t bufSize);
extern "C" int MakeDirWithParent(const char *path, mode_t mode);
extern "C" int MountDir(const char *rootfs, const char *file, unsigned long reMountRwFlag);
extern "C" int DoCtrlDeviceMounting(const char *rootfs);
extern "C" char *GetCgroupMount(char *line, const char *subsys);
extern "C" char *GetCgroupRoot(char *line, const char *subSystem);
extern "C" int ParseFileByLine(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath);
extern "C" int SetupDeviceCgroup(FILE *cgroupAllow, const char *devPath);
extern "C" int SetupDriverCgroup(FILE *cgroupAllow);
extern "C" int GetCgroupPath(const struct CmdArgs *args, char *effPath, const size_t maxSize);
extern "C" int SetupCgroup(const struct ParsedConfig *config);
extern "C" int SetupContainer(struct CmdArgs *args);
extern "C" int Process(int argc, char **argv);
extern "C" int DoFileMounting(const char *rootfs);
extern "C" int DoMounting(const struct ParsedConfig *config);
extern "C" int DoDirectoryMounting(const char *rootfs);
extern "C" int DoPrepare(const struct CmdArgs *args, struct ParsedConfig *config);
extern "C" int ParseRuntimeOptions(const char *options);
extern "C" bool IsOptionNoDrvSet();

struct CmdArgs {
    char     devices[BUF_SIZE];
    char     rootfs[BUF_SIZE];
    int      pid;
    char     options[BUF_SIZE];
};

struct ParsedConfig {
    char rootfs[BUF_SIZE];
    unsigned int devices[MAX_DEVICE_NR];
    size_t devicesNr;
    char containerNsPath[BUF_SIZE];
    char cgroupPath[BUF_SIZE];
    int  originNsFd;
};

int stub_setns(int fd, int nstype)
{
    return 0;
}

int Stub_GetNsPath_Failed(const int pid, const char *nsType, char *buf, size_t bufSize)
{
    return -1;
}

int Stub_GetSelfNsPath_Failed(const char *nsType, char *buf, size_t bufSize)
{
    return -1;
}

int Stub_EnterNsByFd_Success(int fd, int nsType)
{
    return 0;
}

int Stub_EnterNsByFd_Failed(int fd, int nsType)
{
    return -1;
}

int stub_open_success(const char *path, int flags)
{
    return 0;
}

int stub_open_failed(const char *path, int flags)
{
    return -1;
}

int stub_close_success(int fd)
{
    return 0;
}

int stub_MkDir_success(const char *dir, int mode)
{
    return 0;
}

int stub_MkDir_failed(const char *dir, int mode)
{
    return -1;
}

int stub_mount_success(const char *source, const char *target,
                       const char *filesystemtype, unsigned long mountflags, const void *data)
{
    return 0;
}

int stub_Mount_success(const char *src, const char *dst)
{
    return 0;
}

int stub_mount_failed(const char *source, const char *target,
                      const char *filesystemtype, unsigned long mountflags, const void *data)
{
    return -1;
}

int stub_stat_success(const char *file_name, struct stat *buf)
{
    return 0;
}

int stub_stat_failed(const char *file_name, struct stat *buf)
{
    return -1;
}

int Stub_MountDevice_Success(const char *rootfs, const char *deviceName)
{
    return 0;
}

int Stub_MountDevice_Failed(const char *rootfs, const char *deviceName)
{
    return -1;
}

int Stub_MountDir_Success(const char *rootfs, const char *file, unsigned long reMountRwFlag)
{
    return 0;
}

int Stub_MountDir_Failed(const char *rootfs, const char *file, unsigned long reMountRwFlag)
{
    return -1;
}

int Stub_CheckDirExists_Success(char *dir, int len)
{
    return 0;
}

int Stub_MakeDirWithParent_Success(const char *path, mode_t mode)
{
    return 0;
}

int Stub_MakeDirWithParent_Failed(const char *path, mode_t mode)
{
    return -1;
}

int Stub_CheckDirExists_Failed(char *dir, int len)
{
    return -1;
}

int Stub_EnterNsByPath_Success(const char *path, int nsType)
{
    return 0;
}

int Stub_EnterNsByPath_Failed(const char *path, int nsType)
{
    return 0;
}

int Stub_DoDeviceMounting_Success(const char *rootfs, const unsigned int ids[], size_t idsNr)
{
    return 0;
}

int Stub_DoDeviceMounting_Failed(const char *rootfs, const unsigned int ids[], size_t idsNr)
{
    return -1;
}

int Stub_DoCtrlDeviceMounting_Success(const char *rootfs)
{
    return 0;
}

int Stub_DoCtrlDeviceMounting_Failed(const char *rootfs)
{
    return -1;
}

int Stub_DoDirectoryMounting_Success(const char *rootfs)
{
    return 0;
}

int Stub_DoDirectoryMounting_Failed(const char *rootfs)
{
    return -1;
}

int Stub_DoFileMounting_Success(const char *rootfs)
{
    return 0;
}

int Stub_DoFileMounting_Failed(const char *rootfs)
{
    return -1;
}

int Stub_DoMounting_Success(const struct ParsedConfig *config)
{
    return 0;
}

int Stub_DoMounting_Failed(const struct ParsedConfig *config)
{
    return -1;
}

int Stub_SetupCgroup_Success(const struct ParsedConfig *config)
{
    return 0;
}

int Stub_SetupCgroup_Failed(const struct ParsedConfig *config)
{
    return 0;
}

int Stub_SetupContainer_Success(struct CmdArgs *args)
{
    return 0;
}

int Stub_SetupDeviceCgroup_Success(FILE *cgroupAllow, const char *devPath)
{
    return 0;
}

int Stub_SetupDeviceCgroup_Failed(FILE *cgroupAllow, const char *devPath)
{
    return -1;
}

int Stub_SetupDriverCgroup_Fail(FILE *cgroupAllow)
{
    return -1;
}

int Stub_SetupDriverCgroup_Success(FILE *cgroupAllow)
{
    return 0;
}

int Stub_DoPrepare_Failed(const struct CmdArgs *args, struct ParsedConfig *config)
{
    return -1;
}

int Stub_DoPrepare_Success(const struct CmdArgs *args, struct ParsedConfig *config)
{
    return 0;
}

int Stub_ParseFileByLine_Success(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath)
{
    return 0;
}

int Stub_GetCgroupPath_Success(const struct CmdArgs *args, char *effPath, const size_t maxSize)
{
    return 0;
}

bool Stub_IsOptionNoDrvSet_True()
{
    return true;
}

bool Stub_IsOptionNoDrvSet_False()
{
    return false;
}

class Test_Fhho : public Test {
protected:
    static void SetUpTestCase()
    {
        cout << "TestSuite测试套事件：在第一个testcase之前执行" << endl;
    }
    static void TearDownTestCase()
    {
        cout << "TestSuite测试套事件：在最后一个testcase之后执行" << endl;
    }
    //如果想在相同的测试套中设置两种事件，那么可以写在一起，运行就看到效果了
    virtual void SetUp()
    {
        cout << "TestSuite测试用例事件：在每个testcase之前执行" << endl;
    }
    virtual void TearDown()
    {
        cout << "TestSuite测试用例事件：在每个testcase之后执行" << endl;
    }
};

TEST_F(Test_Fhho, ClassEQ1)
{
    EXPECT_EQ(1, IsStrEqual("", ""));
}

TEST_F(Test_Fhho, ClassEQ2)
{
    int pid = 1;
    const char* nsType = "mnt";
    char buf[100] = {0x0};
    int bufSize = 100;
    int ret = GetNsPath(pid, nsType, buf, 100);
    EXPECT_LE(0, ret);
}

TEST(EnterNsByFd, Status1)
{
    int pid = 1;
    int nsType = 1;
    MOCKER(setns)
        .stubs()
        .will(invoke(stub_setns));
    int ret = EnterNsByFd(pid, nsType);
    GlobalMockObject::verify();
    EXPECT_LE(0, ret);
}

TEST(EnterNsByFd, Status2)
{
    int pid = 1;
    int nsType = 1;
    int ret = EnterNsByFd(pid, nsType);
    EXPECT_LE(-1, ret);
}

TEST(EnterNsByPath, Status1)
{
    char containerNsPath[BUF_SIZE] = {0};
    int nsType = 1;
    MOCKER(open).stubs().will(invoke(stub_open_success));
    int ret = EnterNsByPath(containerNsPath, nsType);
    GlobalMockObject::verify();
    EXPECT_LE(-1, ret);
}

TEST(EnterNsByPath, Status2)
{
    char containerNsPath[BUF_SIZE] = {0};
    int nsType = 1;
    int ret = EnterNsByPath(containerNsPath, nsType);
    EXPECT_LE(-1, ret);
}

TEST_F(Test_Fhho, StrHasPrefix)
{
    EXPECT_EQ(1, StrHasPrefix("/home/user", "/home"));
    EXPECT_EQ(0, StrHasPrefix("/home/user", "/heme"));
}

TEST(StrHasPrefix, status1)
{
    EXPECT_EQ(1, StrHasPrefix("/home/user", "/home"));
}

TEST(StrHasPrefix, status2)
{
    EXPECT_EQ(0, StrHasPrefix("/home/user", "/heme"));
}

TEST_F(Test_Fhho, GetNsPath)
{
    char containerNsPath[BUF_SIZE] = {0};
    int containerPid = 1;
    EXPECT_LE(0, GetNsPath(containerPid, "mnt", containerNsPath, BUF_SIZE));
}

TEST_F(Test_Fhho, GetSelfNsPath)
{
    char nsPath[BUF_SIZE] = {0};
    EXPECT_LE(0, GetSelfNsPath("mnt", nsPath, BUF_SIZE));
}


TEST(MountDevice, Status1)
{
    char *rootfs="/home";
    MOCKER(mount).stubs().will(invoke(stub_mount_success));
    MOCKER(stat).stubs().will(invoke(stub_stat_success));
    MOCKER(close).stubs().will(invoke(stub_close_success));
    MOCKER(open).stubs().will(invoke(stub_open_success));
    char *deviceName="davinci100";
    EXPECT_EQ(-1, MountDevice(rootfs, deviceName));
    GlobalMockObject::verify();
}
TEST(MountDevice, Status2)
{
    char *rootfs="/home/notexists";
    char *deviceName="davinci0";
    EXPECT_EQ(-1, MountDevice(rootfs, deviceName));
}

TEST(MountDevice, Status3)
{
    char *rootfs="/home";
    MOCKER(stat).stubs().will(invoke(stub_stat_success));
    MOCKER(close).stubs().will(invoke(stub_close_success));
    MOCKER(open).stubs().will(invoke(stub_open_success));
    MOCKER(mount).stubs().will(invoke(stub_mount_failed));
    char *deviceName="davinci0";
    EXPECT_EQ(-1, MountDevice(rootfs, deviceName));
    GlobalMockObject::verify();
}

TEST(MountDevice, Status4)
{
    char *rootfs="/home";
    MOCKER(mount).stubs().will(invoke(stub_mount_success));
    MOCKER(stat).stubs().will(invoke(stub_stat_failed));
    char *deviceName="davinci0";
    GlobalMockObject::verify();
    EXPECT_EQ(-1, MountDevice(rootfs, deviceName));
}

TEST(DoDeviceMounting, Status1)
{
    MOCKER(MountDevice).stubs().will(invoke(Stub_MountDevice_Success));
    char *rootfs = "/home";
    unsigned int devicesList[2] = {1, 2};
    size_t idNr = 2;
    int ret = DoDeviceMounting(rootfs, devicesList, idNr);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(DoDeviceMounting, Status2)
{
    MOCKER(MountDevice).stubs().will(invoke(Stub_MountDevice_Failed));
    char *rootfs = "/home";
    unsigned int devicesList[2] = {1, 2};
    size_t idNr = 2;
    int ret = DoDeviceMounting(rootfs, devicesList, idNr);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoDirectoryMounting, Status1)
{
    MOCKER(MountDir).stubs().will(invoke(Stub_MountDir_Failed));
    char *rootfs = "/home";
    int ret = DoDirectoryMounting(rootfs);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoDirectoryMounting, Status2)
{
    MOCKER(MountDir).stubs().will(invoke(Stub_MountDir_Success));
    char *rootfs = "/home";
    int ret = DoDirectoryMounting(rootfs);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(DoMounting, Status1)
{
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Failed));
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    int ret = DoMounting(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoMounting, Status2)
{
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Failed));
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    int ret = DoMounting(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoMounting, Status3)
{
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Success));
    MOCKER(DoDirectoryMounting).stubs().will(invoke(Stub_DoDirectoryMounting_Failed));
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    int ret = DoMounting(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoMounting, Status4)
{
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Success));
    MOCKER(IsOptionNoDrvSet).stubs().will(invoke(Stub_IsOptionNoDrvSet_False));
    MOCKER(DoFileMounting).stubs().will(invoke(Stub_DoFileMounting_Success));
    MOCKER(DoDirectoryMounting).stubs().will(invoke(Stub_DoDirectoryMounting_Success));
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    int ret = DoMounting(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}


TEST(CheckDirExists, Status1)
{
    char *dir = "/home";
    int len = strlen(dir);
    int ret = CheckDirExists(dir, len);
    EXPECT_EQ(0, ret);
}

TEST(CheckDirExists, Status2)
{
    char *dir = "/home/notexist";
    int len = strlen(dir);
    int ret = CheckDirExists(dir, len);
    EXPECT_EQ(-1, ret);
}

TEST(GetParentPathStr, Status1)
{
    char *path = "/usr/bin";
    char parent[BUF_SIZE] = {0};
    int ret = GetParentPathStr(path, parent, BUF_SIZE);
    EXPECT_EQ(0, ret);
}

TEST(MakeDirWithParent, Status1)
{
    mode_t mode = 0755;
    char parentDir[BUF_SIZE] = {0};
    int ret = MakeDirWithParent(parentDir, mode);
    EXPECT_EQ(0, ret);
}

TEST(MakeDirWithParent, Status2)
{
    mode_t mode = 0755;
    char parentDir[BUF_SIZE] = {0};
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Success));
    int ret = MakeDirWithParent(parentDir, mode);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(MakeDirWithParent, Status3)
{
    char *pathData = "/path/abc/abcd";
    mode_t mode = 0755;
    char *path = NULL;
    path = strdup(pathData);
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Failed));
    MOCKER(MkDir).stubs().will(invoke(stub_MkDir_success));
    int ret = MakeDirWithParent(path, mode);
    ret = MakeDirWithParent(path, mode);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(MountDir, Status1)
{
    MOCKER(stat).stubs().will(invoke(stub_stat_failed));
    char *rootfs = "/dev";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountDir(rootfs, DAVINCI_MANAGER_PATH, reMountRwFlag);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(MountDir, Status2)
{
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Failed));
    MOCKER(MakeDirWithParent).stubs().will(invoke(Stub_MakeDirWithParent_Failed));
    char *rootfs = "/rootfs";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountDir(rootfs, "/home", reMountRwFlag);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(MountDir, Status3)
{
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Failed));
    MOCKER(MkDir).stubs().will(invoke(stub_MkDir_failed));
    char *rootfs = "/rootfs";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountDir(rootfs, "/home", reMountRwFlag);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(MountDir, Status4)
{
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Failed));
    MOCKER(MakeDirWithParent).stubs().will(invoke(Stub_MakeDirWithParent_Success));
    MOCKER(mount).stubs().will(invoke(stub_mount_failed));
    char *rootfs = "/rootfs";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountDir(rootfs, "/home", reMountRwFlag);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(MountDir, Status5)
{
    MOCKER(CheckDirExists).stubs().will(invoke(Stub_CheckDirExists_Failed));
    MOCKER(MkDir).stubs().will(invoke(stub_MkDir_success));
    MOCKER(Mount).stubs().will(invoke(stub_Mount_success));
    char *rootfs = "/rootfs";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountDir(rootfs, "/dev/random", reMountRwFlag);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(DoCtrlDeviceMounting, Status1)
{
    char *rootfs = "/home";
    MOCKER(MountDir).stubs().will(invoke(Stub_MountDir_Failed));
    int ret = DoCtrlDeviceMounting(rootfs);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoCtrlDeviceMounting, Status2)
{
    MOCKER(MountDevice).stubs().will(invoke(Stub_MountDevice_Success));
    char *rootfs = "/home";
    int ret = DoCtrlDeviceMounting(rootfs);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(GetCgroupMount, Status1)
{
    char *lineData = "406 403 0:27 /docker/ba186404524744c189c6a03d2b66288a963a562a79b11005ae935104fc8c47b2 /sys/fs/cgroup/devices ro,nosuid,nodev,noexec,relatime master:15 - cgroup cgroup rw,devices";
    char *line = NULL;
    line = strdup(lineData);
    char *subsys = "devices";
    char *expectRes = "/sys/fs/cgroup/devices";
    char *actualRes = GetCgroupMount(line, subsys);
    EXPECT_EQ(0, strcmp(actualRes, expectRes));
}

TEST(GetCgroupRoot, Status1)
{
    char *lineData = "3:devices:/docker/ba186404524744c189c6a03d2b66288a963a562a79b11005ae935104fc8c47b2";
    char *line = NULL;
    line = strdup(lineData);
    char *subsys = "devices";
    char *expectRes = "/docker/ba186404524744c189c6a03d2b66288a963a562a79b11005ae935104fc8c47b2";
    char *actualRes = GetCgroupRoot(line, subsys);
    EXPECT_EQ(0, strcmp(actualRes, expectRes));
}

TEST(ParseFileByLine, Status1)
{
    char *mountPath= "/not_exist_dir/mountinfo.txt";
    char mount[BUF_SIZE] = {0x0};
    int ret = ParseFileByLine(mount, BUF_SIZE, GetCgroupMount, mountPath);
    EXPECT_EQ(-1, ret);
}

TEST(SetupDeviceCgroup, Status1)
{
    char *cgroupPathData = "devices.allow";
    char *cgroupPath = NULL;
    cgroupPath = strdup(cgroupPathData);
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    MOCKER(stat).stubs().will(invoke(stub_stat_failed));
    int ret = SetupDeviceCgroup(cgroupAllow, cgroupPath);
    if (cgroupAllow != NULL) {
        fclose(cgroupAllow);
    }
    EXPECT_EQ(-1, ret);
}

TEST(SetupDeviceCgroup, Status2)
{
    char *cgroupPathData = "/not_exist_dir/devices1.allow";
    char *cgroupPath = NULL;
    cgroupPath = strdup(cgroupPathData);
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    MOCKER(stat).stubs().will(invoke(stub_stat_success));
    int ret = SetupDeviceCgroup(cgroupAllow, cgroupPath);
    if (cgroupAllow != NULL) {
        fclose(cgroupAllow);
    }
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupDriverCgroup, Status1)
{
    char *cgroupPath = "devices.allow";
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    MOCKER(SetupDeviceCgroup).stubs().will(invoke(Stub_SetupDeviceCgroup_Success));
    int ret = SetupDriverCgroup(cgroupAllow);
    if (cgroupAllow != NULL) {
        fclose(cgroupAllow);
    }
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(SetupDriverCgroup, Status2)
{
    char *cgroupPath = "devices1.allow";
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    MOCKER(SetupDeviceCgroup).stubs().will(invoke(Stub_SetupDeviceCgroup_Failed));
    int ret = SetupDriverCgroup(cgroupAllow);
    if (cgroupAllow != NULL) {
        fclose(cgroupAllow);
    }
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(GetCgroupPath, Status1)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;

    MOCKER(ParseFileByLine).stubs().will(invoke(Stub_ParseFileByLine_Success));

    char cgroupPath[BUF_SIZE] = {0};
    int ret = GetCgroupPath(&args, cgroupPath, BUF_SIZE);
    EXPECT_EQ(0, ret);
}

TEST(SetupCgroup, Status1)
{
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    strcpy(config.cgroupPath, "/not_exist_dir/cgroup_path");
    int ret = SetupCgroup(&config);
    EXPECT_EQ(-1, ret);
}

TEST(SetupCgroup, Status2)
{
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    strcpy(config.cgroupPath, "devices.allow");
    MOCKER(SetupDriverCgroup).stubs().will(invoke(Stub_SetupDriverCgroup_Fail));
    int ret = SetupCgroup(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupCgroup, Status3)
{
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    strcpy(config.cgroupPath, "devices.allow");
    MOCKER(SetupDriverCgroup).stubs().will(invoke(Stub_SetupDriverCgroup_Success));
    MOCKER(SetupDeviceCgroup).stubs().will(invoke(Stub_SetupDeviceCgroup_Failed));
    int ret = SetupCgroup(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupCgroup, Status4)
{
    MOCKER(SetupDriverCgroup).stubs().will(invoke(Stub_SetupDriverCgroup_Success));
    MOCKER(SetupDeviceCgroup).stubs().will(invoke(Stub_SetupDeviceCgroup_Success));
    struct ParsedConfig config;
    strcpy(config.rootfs, "/home");
    config.devices[0] = 1;
    config.devices[1] = 2;
    config.devicesNr = 2;
    strcpy(config.cgroupPath, "devices.allow");
    int ret = SetupCgroup(&config);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(SetupContainer, Status1)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Failed));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupContainer, Status2)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Success));
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Failed));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupContainer, Status3)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Success));
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoMounting).stubs().will(invoke(Stub_DoMounting_Failed));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupContainer, Status4)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Success));
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoMounting).stubs().will(invoke(Stub_DoMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Failed));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupContainer, Status5)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Success));
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoMounting).stubs().will(invoke(Stub_DoMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Success));
    MOCKER(EnterNsByFd).stubs().will(invoke(Stub_EnterNsByFd_Failed));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupContainer, Status6)
{
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;

    MOCKER(DoPrepare).stubs().will(invoke(Stub_DoPrepare_Success));
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoMounting).stubs().will(invoke(Stub_DoMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Success));
    MOCKER(EnterNsByFd).stubs().will(invoke(Stub_EnterNsByFd_Success));
    int ret = SetupContainer(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(Process, Status1)
{
    int argc = 0;
    char **argv = NULL;
    int ret = Process(argc, argv);
    EXPECT_EQ(-1, ret);
}

TEST(Process, Status2)
{
    int argc = 7;
    const char *argvData[7] = {"ascend-docker-cli", "--devices", "1,2", "--pid", "123", "--rootfs", "/home"};
    int ret = Process(argc,const_cast<char **>(argvData));
    EXPECT_EQ(-1, ret);
}

TEST(Process, Status3)
{
    int argc = 7;
    const char *argvData[7] = {"ascend-docker-cli", "--evices", "1,2", "--idd", "123", "--ootfs", "/home"};
    int ret = Process(argc, const_cast<char **>(argvData));
    EXPECT_EQ(-1, ret);
}

TEST(Process, Status4)
{
    int argc = 7;
    const char *argvData[7] = {"ascend-docker-cli", "--evices", "1,2", "--idd", "123", "--ootfs", "/home"};
    MOCKER(SetupContainer).stubs().will(invoke(Stub_SetupContainer_Success));
    int ret = Process(argc,const_cast<char **>(argvData));
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(ParseRuntimeOptions, Status1)
{
    const char options[BUF_SIZE] = "1,2";
    int ret = ParseRuntimeOptions(options);
    EXPECT_EQ(0, ret);
}

TEST(DoPrepare, Status1)
{
    MOCKER(GetCgroupPath).stubs().will(invoke(Stub_GetCgroupPath_Success));
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    struct ParsedConfig config;
    config.devicesNr = 1024;
    int ret = DoPrepare(&args, &config);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(DoPrepare, Status2)
{
    MOCKER(GetCgroupPath).stubs().will(invoke(Stub_GetCgroupPath_Success));
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    struct ParsedConfig config;
    config.devicesNr = 1024;
    int ret = DoPrepare(&args, &config);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(DoPrepare, Status3)
{
    MOCKER(GetNsPath).stubs().will(invoke(Stub_GetNsPath_Failed));
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    struct ParsedConfig config;
    config.devicesNr = 1024;
    int ret = DoPrepare(&args, &config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoPrepare, Status4)
{
    MOCKER(GetCgroupPath).stubs().will(invoke(Stub_GetCgroupPath_Success));
    MOCKER(GetSelfNsPath).stubs().will(invoke(Stub_GetSelfNsPath_Failed));
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    struct ParsedConfig config;
    config.devicesNr = 1024;
    int ret = DoPrepare(&args, &config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoPrepare, Status5)
{
    MOCKER(GetCgroupPath).stubs().will(invoke(Stub_GetCgroupPath_Success));
    MOCKER(open).stubs().will(invoke(stub_open_failed));
    struct CmdArgs args;
    strcpy(args.rootfs, "/home");
    args.devices[0] = '1';
    args.devices[1] = '2';
    args.pid = 1;
    struct ParsedConfig config;
    config.devicesNr = 1024;
    int ret = DoPrepare(&args, &config);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}
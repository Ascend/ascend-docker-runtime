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
typedef char *(*ParseFileLine)(char *, const char *);
extern "C" int IsStrEqual(const char *s1, const char *s2);
extern "C" int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
extern "C" int setns(int fd, int nstype);
extern "C" int open(const char *path, int flags);
extern "C" int close(int fd);
extern "C" int stat(const char *file_name, struct stat *buf);
extern "C" int mount(const char *source, const char *target,
                     const char *filesystemtype, unsigned long mountflags, const void *data);
extern "C" int rmdir(const char *pathname);
extern "C" int EnterNsByFd(int fd, int nsType);
extern "C" int StrHasPrefix(const char *str, const char *prefix);
extern "C" int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
extern "C" int GetSelfNsPath(const char *nsType, char *buf, size_t bufSize);
extern "C" int EnterNsByPath(const char *path, int nsType);
extern "C" unsigned int GetNextSerialNum();
extern "C" int MountDevice(const char *rootfs, const int serialNumber);
extern "C" int DoDeviceMounting(const char *rootfs, const char *devicesList);
extern "C" int CheckDirExists(char *dir, int len);
extern "C" int GetParentPathStr(const char *path, int lenOfPath, char *parent);
extern "C" int MakeParentDir(char *path, mode_t mode);
extern "C" int MountFiles(const char *rootfs, const char *file, unsigned long reMountRwFlag);
extern "C" int DoCtrlDeviceMounting(const char *rootfs);
extern "C" char *GetCgroupMount(char *line, const char *subsys);
extern "C" char *GetCgroupRoot(char *line, const char *subSystem);
extern "C" int CatFileContent(char* buffer, int bufferSize, ParseFileLine fn, const char* filepath);
extern "C" int SetupDeviceCgroup(FILE *cgroupAllow, const char *devPath);
extern "C" int SetupDriverCgroup(FILE *cgroupAllow);
extern "C" int GetCgroupPath(const struct CmdArgs *args, char *effPath, const size_t maxSize);
extern "C" int SetupCgroup(struct CmdArgs *args, const char *cgroupPath);
extern "C" int SetupMounts(struct CmdArgs *args);
extern "C" void FreeCmdArgs(struct CmdArgs *args);
extern "C" int Process(int argc, char **argv);
extern "C" int DoMounting(const struct CmdArgs *args);

struct CmdArgs {
    char *devices;
    char *rootfs;
    int  pid;
};

int stub_setns(int fd, int nstype)
{
    return 0;
}

int Stub_EnterNsByFd_Success(int fd, int nsType)
{
    return 0;
}

int stub_open(const char *path, int flags)
{
    return 0;
}

int stub_mount_success(const char *source, const char *target,
                       const char *filesystemtype, unsigned long mountflags, const void *data)
{
    return 0;
}

int stub_MountDevice(const char *rootfs, const int serialNumber)
{
    return 0;
}

int stub_MountFiles_success(const char *rootfs, const char *file, unsigned long reMountRwFlag)
{
    return 0;
}


int stub_CheckDirExists_fail(char *dir, int len)
{
    return -1;
}

int Stub_EnterNsByPath_Success(const char *path, int nsType)
{
    return 0;
}

int Stub_DoDeviceMounting_Success(const char *rootfs, const char *devicesList)
{
    return 0;
}

int Stub_DoCtrlDeviceMounting_Success(const char *rootfs)
{
    return 0;
}

int Stub_DoMounting_Success(const struct CmdArgs *args)
{
    return 0;
}

int Stub_SetupCgroup_Success(struct CmdArgs *args, const char *cgroupPath)
{
    return 0;
}

int Stub_SetupMounts_Success(struct CmdArgs *args)
{
    return 0;
}

int Stub_SetupDriverCgroup_Fail(FILE *cgroupAllow)
{
    return 0;
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
    char* nsType = "mnt";
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
    MOCKER(open)
    .stubs()
    .will(invoke(stub_open));
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

TEST_F(Test_Fhho, GetNextSerialNum)
{
    EXPECT_EQ(0, GetNextSerialNum());
    EXPECT_EQ(1, GetNextSerialNum());
    EXPECT_EQ(2, GetNextSerialNum());
}

TEST(MountDevice, Status1)
{
    char *rootfs="/home";
    MOCKER(mount).stubs().will(invoke(stub_mount_success));
    int serialNumber = 0;
    GlobalMockObject::verify();
    EXPECT_EQ(-1, MountDevice(rootfs, serialNumber));
}
TEST(MountDevice, Status11)
{
    char *rootfs="/home/notexists";
    int serialNumber = 0;
    EXPECT_EQ(-1, MountDevice(rootfs, serialNumber));
}

TEST(MountDevice, Status2)
{
    char *rootfs="../testcase/data";
//    char *rootfs="/home/zhouhongyu/DT/ascend-docker-cli/test/build/testcase/data";
    int serialNumber = 0;
    EXPECT_EQ(-1, MountDevice(rootfs, serialNumber));
}

TEST(DoDeviceMounting, Status1)
{
    MOCKER(MountDevice)
        .stubs()
        .will(invoke(stub_MountDevice));
    char *rootfs = "/home";
    char *devicesList = "1,2";
    int ret = DoDeviceMounting(rootfs, devicesList);
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

TEST(GetParentPathStr, Status1)
{
    char *path = "/usr/bin";
    int lenOfPath = strlen(path);
    char parent[BUF_SIZE] = {0};
    int ret = GetParentPathStr(path, lenOfPath, parent);
    EXPECT_EQ(0, ret);
}

TEST(MakeParentDir, Status1)
{
    char *path = "/home/test1/test2";
    mode_t mode = 0755;
    char parentDir[BUF_SIZE] = {0};
    int ret = MakeParentDir(parentDir, mode);
    EXPECT_EQ(0, ret);
}

TEST(MakeParentDir, Status2)
{
    char *pathData = "/home/zhouhongyu/DT/ascend-docker-cli/test/build/abc/abcd";
    mode_t mode = 0755;
    char *path = NULL;
    path = strdup(pathData);
    int ret = MakeParentDir(path, mode);
    rmdir(path);
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/build/abc/");
    EXPECT_EQ(0, ret);
}

TEST(MakeParentDir, Status3)
{
    char *pathData = "/home/zhouhongyu/DT/ascend-docker-cli/test/build/abc/abcd";
    mode_t mode = 0755;
    char *path = NULL;
    path = strdup(pathData);
    int ret = MakeParentDir(path, mode);
    ret = MakeParentDir(path, mode);
    rmdir(path);
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/build/abc/");
    EXPECT_EQ(0, ret);
}

TEST(MountFiles, Status1)
{
    MOCKER(MountDevice)
    .stubs()
    .will(invoke(stub_MountDevice));
    MOCKER(mount).stubs().will(invoke(stub_mount_success));
    char *rootfs = "../testcase/data/dev";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountFiles(rootfs, DAVINCI_MANAGER_PATH, reMountRwFlag);
    umount("../testcase/data/dev" DAVINCI_MANAGER_PATH);
    rmdir("../testcase/data/dev" DAVINCI_MANAGER_PATH);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(MountFiles, Status11)
{
    char *rootfs = "../testcase/data/dev";
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT | MS_RDONLY | MS_NOSUID | MS_NOEXEC;
    int ret = MountFiles(rootfs, DAVINCI_MANAGER_PATH, reMountRwFlag);
    umount("../testcase/data/dev" DAVINCI_MANAGER_PATH);
    rmdir("../testcase/data/dev" DAVINCI_MANAGER_PATH);
    EXPECT_EQ(-1, ret);
}

TEST(MountFiles, Status2)
{
    char *rootfsData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/dev";
    char *rootfs = NULL;
    rootfs = strdup(rootfsData);
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT;
    int ret = MountFiles(rootfs, DAVINCI_MANAGER_PATH, reMountRwFlag);
    umount("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/dev" DAVINCI_MANAGER_PATH);
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/dev" DAVINCI_MANAGER_PATH);
    EXPECT_EQ(-1, ret);
}

TEST(MountFiles, Status3)
{
    char *rootfsData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr";
    char *rootfs = NULL;
    rootfs = strdup(rootfsData);
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT;
    int ret = MountFiles(rootfs, "/usr", reMountRwFlag);
    umount("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    EXPECT_EQ(-1, ret);
}

TEST(MountFiles, Status4)
{
    char *rootfsData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr";
    char *rootfs = NULL;
    rootfs = strdup(rootfsData);
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT;
    int ret = MountFiles(rootfs, "/notexist1/notexist2", reMountRwFlag);
    umount("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    EXPECT_EQ(-1, ret);
}

TEST(MountFiles, Status5)
{
    char *rootfsData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr";
    char *rootfs = NULL;
    rootfs = strdup(rootfsData);
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT;
    int ret = MountFiles(rootfs, "/notexist2", reMountRwFlag);
    umount("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    EXPECT_EQ(-1, ret);
}

TEST(MountFiles, Status6)
{
    MOCKER(CheckDirExists).stubs().will(invoke(stub_CheckDirExists_fail));
    char *rootfsData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr1";
    char *rootfs = NULL;
    rootfs = strdup(rootfsData);
    unsigned long reMountRwFlag = MS_BIND | MS_REMOUNT;
    int ret = MountFiles(rootfs, "/usr", reMountRwFlag);
    umount("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    rmdir("/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/usr");
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(DoCtrlDeviceMounting, Status1)
{
    char *rootfs = "/home";
    int ret = DoCtrlDeviceMounting(rootfs);
    EXPECT_EQ(-1, ret);
}

TEST(DoCtrlDeviceMounting, Status2)
{
    MOCKER(MountFiles).stubs().will(invoke(stub_MountFiles_success));
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

TEST(CatFileContent, Status1)
{
    char *mountPath= "./data/mountinfo.txt";
    char mount[BUF_SIZE] = {0x0};
    int ret = CatFileContent(mount, BUF_SIZE, GetCgroupMount, mountPath);
    EXPECT_EQ(-1, ret);
}

TEST(SetupDeviceCgroup, Status1)
{
    char *cgroupPathData = "../testcase/data/devices.allow";
    char *cgroupPath = NULL;
    cgroupPath = strdup(cgroupPathData);
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    int ret = SetupDeviceCgroup(cgroupAllow, cgroupPath);
    fclose(cgroupAllow);
    EXPECT_EQ(0, ret);
}

TEST(SetupDeviceCgroup, Status2)
{
    char *cgroupPathData = "../testcase/data/devices1.allow";
    char *cgroupPath = NULL;
    cgroupPath = strdup(cgroupPathData);
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    int ret = SetupDeviceCgroup(cgroupAllow, cgroupPath);
    fclose(cgroupAllow);
    EXPECT_EQ(0, ret);
}

TEST(SetupDriverCgroup, Status1)
{
    char *cgroupPath = "../testcase/data/devices.allow";
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    int ret = SetupDriverCgroup(cgroupAllow);
    fclose(cgroupAllow);
    EXPECT_EQ(0, ret);
}

TEST(SetupDriverCgroup, Status2)
{
    char *cgroupPath = "../testcase/data/devices1.allow";
    FILE *cgroupAllow = NULL;
    cgroupAllow = fopen(cgroupPath, "a");
    int ret = SetupDriverCgroup(cgroupAllow);
    fclose(cgroupAllow);
    EXPECT_EQ(0, ret);
}

TEST(GetCgroupPath, Status1)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };

    char cgroupPath[BUF_SIZE] = {0};
    int ret = GetCgroupPath(&args, cgroupPath, BUF_SIZE);
    EXPECT_EQ(0, ret);
}

TEST(SetupCgroup, Status1)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };

    char cgroupPath[BUF_SIZE] = {0};
    int ret = SetupCgroup(&args, cgroupPath);
    EXPECT_EQ(-1, ret);
}

TEST(SetupCgroup, Status2)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    char *cgroupPathData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/data/devices.allow";
    char *cgroupPath = strdup(cgroupPathData);
    int ret = SetupCgroup(&args, cgroupPath);
    EXPECT_EQ(0, ret);
}

TEST(SetupCgroup, Status3)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    char *cgroupPathData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/";
    char *cgroupPath = strdup(cgroupPathData);
    int ret = SetupCgroup(&args, cgroupPath);
    EXPECT_EQ(-1, ret);
}

TEST(SetupCgroup, Status4)
{
    MOCKER(SetupDriverCgroup)
    .stubs()
    .will(invoke(Stub_SetupDriverCgroup_Fail));
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    char *cgroupPathData = "/home/zhouhongyu/DT/ascend-docker-cli/test/testcase/";
    char *cgroupPath = strdup(cgroupPathData);
    int ret = SetupCgroup(&args, cgroupPath);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status1)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    int ret = SetupMounts(&args);
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status2)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status3)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status4)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status5)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status6)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoDeviceMounting).stubs().will(invoke(Stub_DoDeviceMounting_Success));
    MOCKER(DoCtrlDeviceMounting).stubs().will(invoke(Stub_DoCtrlDeviceMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Success));
    MOCKER(EnterNsByFd).stubs().will(invoke(Stub_EnterNsByFd_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}

TEST(SetupMounts, Status7)
{
    struct CmdArgs args = {
            .devices = "1,2",
            .rootfs  = "/home",
            .pid     = 1
    };
    MOCKER(EnterNsByPath).stubs().will(invoke(Stub_EnterNsByPath_Success));
    MOCKER(DoMounting).stubs().will(invoke(Stub_DoMounting_Success));
    MOCKER(SetupCgroup).stubs().will(invoke(Stub_SetupCgroup_Success));
    MOCKER(EnterNsByFd).stubs().will(invoke(Stub_EnterNsByFd_Success));
    int ret = SetupMounts(&args);
    GlobalMockObject::verify();
    EXPECT_EQ(0, ret);
}

TEST(FreeCmdArgs, Status1)
{
    struct CmdArgs args = {
            .devices = NULL,
            .rootfs  = NULL,
            .pid     = 1
    };
    FreeCmdArgs(&args);
}

TEST(FreeCmdArgs, Status2)
{
    struct CmdArgs args = {
            .devices = NULL,
            .rootfs  = NULL,
            .pid     = 1
    };
    char *devlist = "1,2";
    char *root = "/home";
    args.devices = strdup(devlist);
    args.rootfs = strdup(root);
    FreeCmdArgs(&args);
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
    char *argvData[7] = {"ascend-docker-cli", "--devices", "1,2", "--pid", "123", "--rootfs", "/home"};
    int ret = Process(argc, argvData);
    EXPECT_EQ(-1, ret);
}

TEST(Process, Status3)
{
    int argc = 7;
    char *argvData[7] = {"ascend-docker-cli", "--evices", "1,2", "--idd", "123", "--ootfs", "/home"};
    int ret = Process(argc, argvData);
    EXPECT_EQ(-1, ret);
}

TEST(Process, Status4)
{
    int argc = 7;
    char *argvData[7] = {"ascend-docker-cli", "--evices", "1,2", "--idd", "123", "--ootfs", "/home"};
    MOCKER(SetupMounts).stubs().will(invoke(Stub_SetupMounts_Success));
    int ret = Process(argc, argvData);
    GlobalMockObject::verify();
    EXPECT_EQ(-1, ret);
}
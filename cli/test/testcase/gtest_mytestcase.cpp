// Demo.cpp : Defines the entry point for the console application.
//
#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
//建议这样引用，避免下面用关键字时需要加前缀 testing::
using namespace testing;

extern "C" int IsStrEqual(const char *s1, const char *s2);  
extern "C" int GetNsPath(const int pid, const char *nsType, char *buf, size_t bufSize);
extern "C" int setns(int fd, int nstype);
extern "C" int EnterNsByFd(int fd, int nsType);

int stub_setns(int fd, int nstype)
{
     return 0;
}

class Test_Fhho : public Test
{
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

#if 0
TEST_F(Test_Fhho, ClassEQ2)
{   
   int pid = 1;
   char* nsType = "mnt";
   char buf[100] = {0x0}; 
   int bufSize = 100;
   int ret = GetNsPath(pid, nsType, buf, 100);
   EXPECT_EQ(1, ret);
}

TEST_F(Test_Fhho, ClassEQ3)
{
    int pid = 1;
    int nsType = 1;
    MOCKER(setns)
        .stubs()
        .will(invoke(stub_setns));
    int ret = EnterNsByFd(pid, nsType);
    EXPECT_EQ(1, ret);
}
#endif

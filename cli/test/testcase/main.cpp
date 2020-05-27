#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

//#include "gtest_testcase.cpp"
//#include "mockcpp_testcase.cpp"

using namespace std;
//建议这样引用，避免下面用关键字时需要加前缀 testing::
using namespace testing;


int main(int argc, char* argv[], char* evn[])
{
	//std::vector<PFUNC> g_func1 = GET_FUNC_CTOR_LIST();
	//全局事件：设置执行全局事件
        //ddGlobalTestEnvironment(new FooEnvironment);
    
	//输出 用例列表，用例不执行了~
	//testing::GTEST_FLAG(list_tests) = " ";
	//设置过滤功能后，参数化功能失效~~~~//执行列出来的测试套的用例
	//testing::GTEST_FLAG(filter) = "EXEPath.*";//"FooTest.*:TestCase.*:TestSuite.*:TestCaseTest.*:IsPrimeParamTest.*";

	//测试套排序，下面两种情况不能同时使用，否则排序就无作用
	//GTEST_FLAG(list_order) = "Test_Fhho;UT_DEMO;TestSuitName;FuncFoo;TestSuitEvent";
	//测试套模糊匹配排序，注：只以开头进行精确匹配，遇到 * 后模糊匹配
	//如UT_*;IT_*，先执行所有UT_开头的用例再执行IT_开头的用例
	/*GTEST_FLAG(dark_list_order) = "UT_*;\
		    IT_*";*/
    // Returns 0 if all tests passed, or 1 other wise.
	int ret = Init_UT(argc, argv, true); 
    if (1 == ret)
    {
        printf("有用例错误，请按任意键继续。。。");
        //getchar();
    }

    return ret;   
} 

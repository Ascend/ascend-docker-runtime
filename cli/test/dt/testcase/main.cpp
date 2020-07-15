/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: 测试框架主函数
*/
#include <string>
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace testing;

int main(int argc, char* argv[], char* evn[])
{
    int ret = Init_UT(argc, argv, true); 
    if (1 == ret) {
        printf("有用例错误，请按任意键继续。。。");
    }

    return ret;
} 

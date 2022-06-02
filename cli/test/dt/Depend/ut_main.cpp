/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: 测试框架
*/
#include "gtest/gtest.h"
#include <stdio.h>

using namespace testing;


int main(int argc, char* argv[])
{
    InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    return result;
}

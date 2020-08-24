/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: 测试框架
*/
#ifndef __UT_COMM_H__
#define __UT_COMM_H__

#include <stdio.h>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"


#define EXTERN_C extern "C"

#define ENVIRONMENT_AUTO_REGISTER(Name, Environment) \
namespace { \
class Name##AutoRegister { \
public: \
    Name##AutoRegister() { \
        testing::AddGlobalTestEnvironment(new Environment); \
    } \
}; \
Name##AutoRegister global##Name; \
}


#endif

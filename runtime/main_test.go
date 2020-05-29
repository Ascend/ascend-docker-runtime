/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Runtime DT测试
 */
package main

import (
	"os"
	"testing"

	"github.com/prashantv/gostub"
)

func TestArgsIsCreate(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle", "."}
	stub := gostub.Stub(&os.Args, testArgs)
	defer stub.Reset()

	stub.Stub(&execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	if err := doProcess(); err != nil {
		t.Fatalf("%v", err)
	}
}

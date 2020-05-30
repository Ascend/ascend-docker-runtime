/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Runtime DT测试
 */
package main

import (
	"os"
	"testing"

	"github.com/opencontainers/runtime-spec/specs-go"
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

func TestArgsIsCreateCase1(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle"}
	stub := gostub.Stub(&os.Args, testArgs)
	defer stub.Reset()

	stub.Stub(&execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	if err := doProcess(); err == nil {
		t.Fatalf("%v", err)
	}
}

func TestArgsIsCreateCase2(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle", ""}
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

func TestArgsIsCreateCase3(t *testing.T) {
	t.Log("进入测试用例")

	if err := os.Mkdir("./test", 0655); err != nil {
	}
	f, err := os.Create("./test/config.json")
	defer f.Close()
	if err != nil {
	}
	testArgs := []string{"create", "--bundle", "./test"}
	stub := gostub.Stub(&os.Args, testArgs)
	defer stub.Reset()

	stub.Stub(&execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	if err := doProcess(); err == nil {
		t.Fatalf("%v", err)
	}
}

func TestArgsIsCreateCase4(t *testing.T) {
	t.Log("进入测试用例")

	if err := os.Mkdir("./test", 0655); err != nil {
	}
	f, err := os.Create("./test/config.json")
	defer f.Close()
	if err != nil {
	}
	testArgs := []string{"spec", "--bundle", "./test"}
	stub := gostub.Stub(&os.Args, testArgs)
	defer stub.Reset()

	stub.Stub(&execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	if err := doProcess(); err == nil {
		t.Fatalf("%v", err)
	}
}

func TestModifySpecFile(t *testing.T) {
	if err := modifySpecFile("./test/config.json"); err != nil {
		t.Log("run modifySpecFile failed")
	}
}

func TestModifySpecFileCase1(t *testing.T) {
	var file string = "./test"
	if err := os.Mkdir("./test", 0400); err != nil {

	}

	if err := modifySpecFile(file); err != nil {
		t.Log("run modifySpecFile failed")
	}
	if err := os.Remove(file); err != nil {

	}
}

func TestModifySpecFileCase2(t *testing.T) {
	var file string = "./test.json"
	f, err := os.Create(file)
	defer f.Close()
	if err != nil {
		t.Log("create file error")
	}

	if err := modifySpecFile(file); err != nil {
		t.Log("run modifySpecFile failed")
	}
	if err := os.Remove(file); err != nil {

	}
}

func TestModifySpecFileCase3(t *testing.T) {
	var file string = "./test_spec.json"
	if err := modifySpecFile(file); err != nil {
		t.Log("run modifySpecFile failed")
	}
}

func TestAddHook(t *testing.T) {
	var specArgs = &specs.Spec{}
	if err := addHook(specArgs); err != nil {
	}
}

func TestAddHookCase1(t *testing.T) {
	var specArgs = &specs.Spec{}
	stub := gostub.Stub(&hookCliPath, ".")
	defer stub.Reset()

	if err := addHook(specArgs); err != nil {
	}
}

func TestAddHookCase2(t *testing.T) {
	var specArgs = &specs.Spec{}
	stub := gostub.Stub(&hookCliPath, ".")
	defer stub.Reset()
	stub.Stub(&hookDefaultFile, ".")
	if err := addHook(specArgs); err != nil {
	}
}

func TestAddHookCase3(t *testing.T) {
	var file string = "/usr/local/bin/ascend-docker-hook"
	var filenew string = "/usr/local/bin/ascend-docker-hook-1"

	if err := os.Rename(file, filenew); err != nil {
		t.Log("rename ", file)
	}
	var specArgs = &specs.Spec{}
	if err := addHook(specArgs); err != nil {
	}
	if err := os.Rename(filenew, file); err != nil {
		t.Log("rename ", file)
	}
}

func TestExecRunc(t *testing.T) {
	stub := gostub.Stub(&dockerRuncName, "abc-runc")
	stub.Stub(&runcName, "runc123")
	defer stub.Reset()

	if err := execRunc(); err != nil {

	}
}
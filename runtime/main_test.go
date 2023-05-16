/* Copyright(C) 2022. Huawei Technologies Co.,Ltd. All rights reserved.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

// Package main
package main

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"testing"

	"github.com/agiledragon/gomonkey/v2"
	"github.com/containerd/containerd/oci"
	"github.com/opencontainers/runtime-spec/specs-go"
	"github.com/stretchr/testify/assert"

	"main/dcmi"
)

func TestArgsIsCreate(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle", "."}
	stub := gomonkey.ApplyGlobalVar(&os.Args, testArgs)
	defer stub.Reset()

	stub.ApplyFunc(execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	err := doProcess()
	assert.NotNil(t, err)
}

func TestArgsIsCreateCase1(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle"}
	stub := gomonkey.ApplyGlobalVar(&os.Args, testArgs)
	defer stub.Reset()

	stub.ApplyFunc(execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	err := doProcess()
	assert.NotNil(t, err)
}

func TestArgsIsCreateCase2(t *testing.T) {
	t.Log("进入测试用例")

	testArgs := []string{"create", "--bundle", ""}
	stub := gomonkey.ApplyGlobalVar(&os.Args, testArgs)
	defer stub.Reset()

	stub.ApplyFunc(execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	err := doProcess()
	assert.NotNil(t, err)
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
	stub := gomonkey.ApplyGlobalVar(&os.Args, testArgs)
	defer stub.Reset()

	stub.ApplyFunc(execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	err = doProcess()
	assert.NotNil(t, err)
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
	stub := gomonkey.ApplyGlobalVar(&os.Args, testArgs)
	defer stub.Reset()

	stub.ApplyFunc(execRunc, func() error {
		t.Log("execute stub")
		return nil
	})

	err = doProcess()
	assert.Nil(t, err)
}

func TestModifySpecFile(t *testing.T) {
	err := modifySpecFile("./test/config.json")
	assert.NotNil(t, err)
}

func TestModifySpecFileCase1(t *testing.T) {
	file := "./test"
	if err := os.Mkdir("./test", 0400); err != nil {

	}

	err := modifySpecFile(file)
	assert.NotNil(t, err)
	if err := os.Remove(file); err != nil {

	}
}

func TestModifySpecFileCase2(t *testing.T) {
	file := "./test.json"
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
	file := "./test_spec.json"
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
	stub := gomonkey.ApplyGlobalVar(&hookCliPath, ".")
	defer stub.Reset()

	err := addHook(specArgs)
	assert.NotNil(t, err)
}

func TestAddHookCase2(t *testing.T) {
	var specArgs = &specs.Spec{}
	stub := gomonkey.ApplyGlobalVar(&hookCliPath, ".")
	defer stub.Reset()
	stub.ApplyGlobalVar(&hookDefaultFile, ".")
	err := addHook(specArgs)
	assert.NotNil(t, err)
}

func TestAddHookCase3(t *testing.T) {
	file := "/usr/local/bin/ascend-docker-hook"
	filenew := "/usr/local/bin/ascend-docker-hook-1"

	if err := os.Rename(file, filenew); err != nil {
		t.Log("rename ", file)
	}
	var specArgs = &specs.Spec{}
	err := addHook(specArgs)
	assert.NotNil(t, err)

	if err := os.Rename(filenew, file); err != nil {
		t.Log("rename ", file)
	}
}

func TestExecRunc(t *testing.T) {
	stub := gomonkey.ApplyGlobalVar(&dockerRuncName, "abc-runc")
	stub.ApplyGlobalVar(&runcName, "runc123")
	defer stub.Reset()

	err := execRunc()
	assert.NotNil(t, err)
}

func TestParseDevicesCase1(t *testing.T) {
	visibleDevices := "0-3,5,7"
	expectVal := []int{0, 1, 2, 3, 5, 7}
	actualVal, err := parseDevices(visibleDevices)
	if err != nil || !reflect.DeepEqual(expectVal, actualVal) {
		t.Fail()
	}
}

func TestParseDevicesCase2(t *testing.T) {
	visibleDevices := "0-3-4,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestParseDevicesCase3(t *testing.T) {
	visibleDevices := "0l-3,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestParseDevicesCase4(t *testing.T) {
	visibleDevices := "0-3o,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestParseDevicesCase5(t *testing.T) {
	visibleDevices := "4-3,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestParseDevicesCase6(t *testing.T) {
	visibleDevices := "3o,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestParseDevicesCase7(t *testing.T) {
	visibleDevices := "0=3,5,7"
	_, err := parseDevices(visibleDevices)
	assert.NotNil(t, err)
}

func TestRemoveDuplication(t *testing.T) {
	originList := []int{1, 2, 2, 4, 5, 5, 5, 6, 8, 8}
	targetList := []int{1, 2, 4, 5, 6, 8}
	resultList := removeDuplication(originList)

	assert.EqualValues(t, targetList, resultList)
}

func TestAddEnvToDevicePlugin0(t *testing.T) {
	devicePluginHostName := devicePlugin + "pf2i6r"
	spec := specs.Spec{
		Process: &specs.Process{
			Env: []string{"KUBE_DNS_PORT_53_UDP_PORT=53",
				fmt.Sprintf("HOSTNAME=%s", devicePluginHostName),
				"KUBE_DNS_PORT_53_UDP_PROTO=udp"},
		},
	}

	addEnvToDevicePlugin(&spec)
	assert.Contains(t, spec.Process.Env, useAscendDocker)
}

func TestAddEnvToDevicePlugin1(t *testing.T) {
	devicePluginHostName := "pf2i6r"
	spec := specs.Spec{
		Process: &specs.Process{
			Env: []string{"KUBE_DNS_PORT_53_UDP_PORT=53",
				fmt.Sprintf("HOSTNAME=%s", devicePluginHostName),
				"KUBE_DNS_PORT_53_UDP_PROTO=udp"},
		},
	}

	addEnvToDevicePlugin(&spec)
	assert.NotContains(t, spec.Process.Env, useAscendDocker)
}

func TestGetDeviceTypeByChipName0(t *testing.T) {
	chipName := "310B"
	devType := GetDeviceTypeByChipName(chipName)
	assert.EqualValues(t, Ascend310B, devType)
}

func TestGetDeviceTypeByChipName1(t *testing.T) {
	chipName := "310P"
	devType := GetDeviceTypeByChipName(chipName)
	assert.EqualValues(t, Ascend310P, devType)
}

func TestGetDeviceTypeByChipName2(t *testing.T) {
	chipName := "310"
	devType := GetDeviceTypeByChipName(chipName)
	assert.EqualValues(t, Ascend310, devType)
}

func TestGetDeviceTypeByChipName3(t *testing.T) {
	chipName := "910"
	devType := GetDeviceTypeByChipName(chipName)
	assert.EqualValues(t, Ascend910, devType)
}

func TestGetDeviceTypeByChipName4(t *testing.T) {
	chipName := "980b"
	devType := GetDeviceTypeByChipName(chipName)
	assert.EqualValues(t, "", devType)
}

func TestGetValueByKeyCase1(t *testing.T) {
	data := []string{"ASCEND_VISIBLE_DEVICES=0-3,5,7"}
	word := "ASCEND_VISIBLE_DEVICES"
	expectVal := "0-3,5,7"
	actualVal := getValueByKey(data, word)
	assert.EqualValues(t, expectVal, actualVal)
}

func TestGetValueByKeyCase2(t *testing.T) {
	data := []string{"ASCEND_VISIBLE_DEVICES"}
	word := "ASCEND_VISIBLE_DEVICES"
	expectVal := ""
	defer func() {
		if err := recover(); err != nil {
			t.Log("exception occur")
		}
	}()
	actualVal := getValueByKey(data, word)
	assert.EqualValues(t, expectVal, actualVal)
}

func TestGetValueByKeyCase3(t *testing.T) {
	data := []string{"ASCEND_VISIBLE_DEVICES=0-3,5,7"}
	word := "ASCEND_VISIBLE_DEVICE"
	expectVal := ""
	actualVal := getValueByKey(data, word)
	assert.EqualValues(t, expectVal, actualVal)
}

func TestUpdateEnvAndPostHook(t *testing.T) {
	defer func() {
		if e := recover(); e != nil {
			t.Logf("%s", e)
		}
	}()
	vdvice := dcmi.VDeviceInfo{
		CardID:    0,
		DeviceID:  0,
		VdeviceID: 100,
	}

	spec := specs.Spec{
		Process: &specs.Process{
			Env: []string{"KUBE_DNS_PORT_53_UDP_PORT=53",
				fmt.Sprintf("%s=0", ascendVisibleDevices),
				"KUBE_DNS_PORT_53_UDP_PROTO=udp"},
		},
		Hooks: &specs.Hooks{},
	}

	updateEnvAndPostHook(&spec, vdvice)
	assert.Contains(t, spec.Process.Env, "ASCEND_VISIBLE_DEVICES=100")
	assert.Contains(t, spec.Process.Env, "ASCEND_RUNTIME_OPTIONS=VIRTUAL")
	assert.Contains(t, spec.Hooks.Poststop[0].Path, destroyHookCli)
}

func TestAddDeviceToSpec0(t *testing.T) {
	devPath := "/dev/davinci0"
	statStub := gomonkey.ApplyFunc(oci.DeviceFromPath, func(name string) (*specs.LinuxDevice, error) {
		return &specs.LinuxDevice{
			Path: devPath,
		}, nil
	})
	defer statStub.Reset()

	spec := specs.Spec{
		Linux: &specs.Linux{
			Devices: []specs.LinuxDevice{},
			Resources: &specs.LinuxResources{
				Devices: []specs.LinuxDeviceCgroup{},
			},
		},
	}

	err := addDeviceToSpec(&spec, devPath, false)
	assert.Nil(t, err)
	assert.Contains(t, spec.Linux.Devices[0].Path, devPath)
}

func TestAddAscend310BManagerDevice(t *testing.T) {
	statStub := gomonkey.ApplyFunc(addDeviceToSpec, func(spec *specs.Spec, dPath string, vdevice bool) error {
		return nil
	})
	defer statStub.Reset()

	spec := specs.Spec{
		Linux: &specs.Linux{
			Devices: []specs.LinuxDevice{},
			Resources: &specs.LinuxResources{
				Devices: []specs.LinuxDeviceCgroup{},
			},
		},
	}

	err := addAscend310BManagerDevice(&spec)
	assert.Nil(t, err)
}

func TestAddCommonManagerDevice(t *testing.T) {
	statStub := gomonkey.ApplyFunc(addDeviceToSpec, func(spec *specs.Spec, dPath string, vdevice bool) error {
		return nil
	})
	defer statStub.Reset()

	spec := specs.Spec{
		Linux: &specs.Linux{
			Devices: []specs.LinuxDevice{},
			Resources: &specs.LinuxResources{
				Devices: []specs.LinuxDeviceCgroup{},
			},
		},
	}

	err := addCommonManagerDevice(&spec)
	assert.Nil(t, err)
}

func TestAddManagerDevice(t *testing.T) {
	devPath := "/dev/mockdevice"
	statStub := gomonkey.ApplyFunc(oci.DeviceFromPath, func(dPath string) (*specs.LinuxDevice, error) {
		return &specs.LinuxDevice{
			Path: devPath,
		}, nil
	})
	defer statStub.Reset()

	dcmiStub := gomonkey.ApplyFunc(dcmi.GetChipName, func() (string, error) {
		return "910", nil
	})
	defer dcmiStub.Reset()

	productStub := gomonkey.ApplyFunc(dcmi.GetProductType, func(w dcmi.WorkerInterface) (string, error) {
		return "", nil
	})
	defer productStub.Reset()

	spec := specs.Spec{
		Linux: &specs.Linux{
			Devices: []specs.LinuxDevice{},
			Resources: &specs.LinuxResources{
				Devices: []specs.LinuxDeviceCgroup{},
			},
		},
	}
	ctx, _ := context.WithCancel(context.Background())
	err := initLogModule(ctx)
	assert.Nil(t, err)
	err = addManagerDevice(&spec)
	assert.Nil(t, err)
}

func TestAddDevice(t *testing.T) {
	devPath := "/dev/davinci1"
	statStub := gomonkey.ApplyFunc(oci.DeviceFromPath, func(name string) (*specs.LinuxDevice, error) {
		return &specs.LinuxDevice{
			Path: devPath,
		}, nil
	})
	defer statStub.Reset()

	manageDeviceStub := gomonkey.ApplyFunc(addManagerDevice, func(spec *specs.Spec) error {
		return nil
	})
	defer manageDeviceStub.Reset()

	spec := specs.Spec{
		Linux: &specs.Linux{
			Devices: []specs.LinuxDevice{},
			Resources: &specs.LinuxResources{
				Devices: []specs.LinuxDeviceCgroup{},
			},
		},
		Process: &specs.Process{
			Env: []string{"KUBE_DNS_PORT_53_UDP_PORT=53",
				"ASCEND_VISIBLE_DEVICES=1",
				"ASCEND_RUNTIME_OPTIONS=",
				"KUBE_DNS_PORT_53_UDP_PROTO=udp"},
		},
	}

	ctx, _ := context.WithCancel(context.Background())
	err := initLogModule(ctx)
	assert.Nil(t, err)
	err = addDevice(&spec)
	assert.Nil(t, err)
	assert.Contains(t, spec.Linux.Devices[0].Path, devPath)
}

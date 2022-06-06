// Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.

// Description: dcmi DT测试
package dcmi

import (
	"testing"

	"github.com/opencontainers/runtime-spec/specs-go"
)

const mockDeviceId = 100

type mockWorker struct{}

func (w *mockWorker) Initialize() error {
	return nil
}

// ShutDown shutdown mock lib
func (w *mockWorker) ShutDown() {
	return
}

// CreateVDevice create v device
func (w *mockWorker) CreateVDevice(_, _ int32, _ string) (int32, error) {

	return int32(mockDeviceId), nil
}

// DestroyVDevice destroy virtual device
func (w *mockWorker) DestroyVDevice(_, _, _ int32) error {
	return nil
}

// FindDevice find device by phyical id
func (w *mockWorker) FindDevice(_ int32) (int32, int32, error) {
	return 0, 0, nil
}

func TestCreateVDevice(t *testing.T) {
	t.Log("TestCreateVDevice start")
	process := specs.Process{}
	spec := specs.Spec{Process: &process}
	spec.Process.Env = []string{}

	// no split, all ok
	vdevice, err := CreateVDevice(&mockWorker{}, &spec)
	if err != nil {
		t.Fatalf("%v %v", vdevice, err)
	}

	// no npu assigin for split
	spec.Process.Env = []string{"ASCEND_VNPU_SPECS=vir04"}
	vdevice, err = CreateVDevice(&mockWorker{}, &spec)
	if err == nil {
		t.Fatalf("%v %v", vdevice, err)
	}

	// split ok
	spec.Process.Env = []string{"ASCEND_VNPU_SPECS=vir04", "ASCEND_VISIBLE_DEVICES=0"}
	vdevice, err = CreateVDevice(&mockWorker{}, &spec)
	if err != nil {
		t.Fatalf("%v %v", vdevice, err)
	}
	if vdevice.VdeviceID != mockDeviceId {
		t.Fatalf("%v %v", vdevice, err)
	}

}

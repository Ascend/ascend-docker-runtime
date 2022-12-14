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

// Description: dcmi DT Test
package dcmi

import (
	"testing"

	"github.com/opencontainers/runtime-spec/specs-go"
)

const mockDeviceID = 100

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

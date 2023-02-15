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

// Package dcmi is used to work with Ascend devices
package dcmi

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/opencontainers/runtime-spec/specs-go"
)

// VDeviceInfo vdevice created info
type VDeviceInfo struct {
	CardID    int32
	DeviceID  int32
	VdeviceID int32
}

// WorkerInterface worker interface
type WorkerInterface interface {
	Initialize() error
	ShutDown()
	FindDevice(visibleDevice int32) (int32, int32, error)
	CreateVDevice(cardID, deviceID int32, coreNum string) (int32, error)
	DestroyVDevice(cardID, deviceID int32, vDevID int32) error
}

// CreateVDevice will create virtual device
func CreateVDevice(w WorkerInterface, spec *specs.Spec) (VDeviceInfo, error) {
	visibleDevice, splitDevice, err := extractVpuParam(spec)
	invalidVDevice := VDeviceInfo{CardID: -1, DeviceID: -1, VdeviceID: -1}
	if err != nil || visibleDevice < 0 {
		return invalidVDevice, err
	}
	if err := w.Initialize(); err != nil {
		return invalidVDevice, fmt.Errorf("cannot init dcmi : %v", err)
	}
	defer w.ShutDown()
	targetDeviceID, targetCardID, err := w.FindDevice(visibleDevice)
	if err != nil {
		return invalidVDevice, err
	}

	vdeviceID, err := w.CreateVDevice(targetCardID, targetDeviceID, splitDevice)
	if err != nil || vdeviceID < 0 {
		return invalidVDevice, fmt.Errorf("cannot create vd or vdevice is wrong: %v %v", vdeviceID, err)
	}
	return VDeviceInfo{CardID: targetCardID, DeviceID: targetDeviceID, VdeviceID: int32(vdeviceID)}, nil
}

func extractVpuParam(spec *specs.Spec) (int32, string, error) {
	splitDevice, needSplit, visibleDeviceLine := "", false, ""
	allowSplit := map[string]string{
		"vir01": "vir01", "vir02": "vir02", "vir04": "vir04", "vir08": "vir08", "vir16": "vir16",
		"vir04_3c": "vir04_3c", "vir02_1c": "vir02_1c", "vir04_4c_dvpp": "vir04_4c_dvpp",
		"vir04_3c_ndvpp": "vir04_3c_ndvpp",
	}

	for _, line := range spec.Process.Env {
		words := strings.Split(line, "=")
		const LENGTH int = 2
		if len(words) != LENGTH {
			continue
		}
		if strings.TrimSpace(words[0]) == "ASCEND_VISIBLE_DEVICES" {
			visibleDeviceLine = words[1]
		}
		if strings.TrimSpace(words[0]) == "ASCEND_VNPU_SPECS" {
			if split, ok := allowSplit[words[1]]; split != "" && ok {
				splitDevice = split
				needSplit = true
			} else {
				return -1, "", fmt.Errorf("cannot parse param : %v", words[1])
			}
		}
	}
	if !needSplit {
		return -1, "", nil
	}
	visibleDevice, err := strconv.Atoi(visibleDeviceLine)
	if err != nil || visibleDevice < 0 || visibleDevice >= hiAIMaxCardNum*hiAIMaxDeviceNum {
		return -1, "", fmt.Errorf("cannot parse param : %v %s", err, visibleDeviceLine)

	}

	return int32(visibleDevice), splitDevice, nil
}

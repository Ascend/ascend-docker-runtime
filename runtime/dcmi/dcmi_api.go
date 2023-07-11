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

	"huawei.com/npu-exporter/v5/common-utils/hwlog"
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
	GetProductType(cardID, deviceID int32) (string, error)
	GetChipInfo(cardID, deviceID int32) (*ChipInfo, error)
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
		"vir04_3c_ndvpp": "vir04_3c_ndvpp", "vir12_3c_32g": "vir12_3c_32g", "vir12_3c_32g_m": "vir12_3c_32g_m",
		"vir12_3c_32g_nm": "vir12_3c_32g_nm", "vir06_1c_16g": "vir06_1c_16g", "vir03_hc_8g": "vir03_hc_8g",
		"vir10_3c_16g": "vir10_3c_16g", "vir10_3c_16g_m": "vir10_3c_16g_m",
		"vir10_3c_16g_nm": "vir10_3c_16g_nm", "vir05_1c_8g": "vir05_1c_8g",
		"vir10_3c_32g": "vir10_3c_32g", "vir10_3c_32g_m": "vir10_3c_32g_m", "vir10_3c_32g_nm": "vir10_3c_32g_nm",
		"vir05_1c_16g": "vir05_1c_16g",
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

// GetProductType get type of product
func GetProductType(w WorkerInterface) (string, error) {
	invalidType := ""
	if err := w.Initialize(); err != nil {
		return invalidType, fmt.Errorf("cannot init dcmi : %v", err)
	}
	defer w.ShutDown()

	cardNum, cardList, err := GetCardList()
	if cardNum == 0 || err != nil {
		hwlog.RunLog.Errorf("failed to get card list, err: %#v", err)
		return invalidType, err
	}
	for _, cardID := range cardList {
		devNum, err := GetDeviceNumInCard(cardID)
		if err != nil {
			hwlog.RunLog.Debugf("get device num by cardID(%d) failed, error: %#v", cardID, err)
			continue
		}
		if devNum == 0 {
			hwlog.RunLog.Debugf("not found device on card %d", cardID)
			continue
		}
		for devID := int32(0); devID < devNum; devID++ {
			productType, err := w.GetProductType(cardID, devID)
			if err != nil {
				hwlog.RunLog.Debugf("get product type by card %d deviceID %d failed, err: %#v", cardID, devID, err)
				continue
			}
			return productType, nil
		}
	}

	return invalidType, nil
}

// GetChipName get name of chip
func GetChipName() (string, error) {
	dcWorker := NpuWorker{}
	invalidName := ""

	if err := dcWorker.Initialize(); err != nil {
		return invalidName, fmt.Errorf("cannot init dcmi : %v", err)
	}
	defer dcWorker.ShutDown()

	cardNum, cardList, err := GetCardList()
	if err != nil {
		hwlog.RunLog.Errorf("failed to get card list, err: %#v", err)
		return invalidName, err
	}
	if cardNum == 0 {
		return invalidName, fmt.Errorf("get chip info failed, no card found")
	}

	// get device in card, then get chip info by cardID and deviceID
	for _, cardID := range cardList {
		devNum, err := GetDeviceNumInCard(cardID)
		if err != nil || devNum == 0 {
			hwlog.RunLog.Warnf("get device num by cardID(%d) failed, error: %#v", cardID, err)
			continue
		}
		for devID := int32(0); devID < devNum; devID++ {
			chipInfo, err := dcWorker.GetChipInfo(cardID, devID)
			if err != nil {
				hwlog.RunLog.Warnf("get chip info failed by cardID(%d), deviceID(%d), error: %#v", cardID, devID,
					err)
				continue
			}
			if !isValidChipInfo(chipInfo) {
				hwlog.RunLog.Warnf("invalid chip info by cardID(%d), deviceID(%d), error: %#v", cardID, devID,
					err)
				continue
			}
			return (*chipInfo).Name, nil
		}
	}

	return invalidName, fmt.Errorf("cannot get valid chip info")
}

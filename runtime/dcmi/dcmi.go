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

// Package dcmi
package dcmi

// #cgo LDFLAGS: -ldl
// #include "dcmi_interface_api.h"
import "C"
import (
	"fmt"
	"math"
	"unsafe"

	"mindxcheckutils"
)

const (
	// RetError return error when the function failed
	retError = -1
	// hiAIMaxCardNum is the max number of cards
	hiAIMaxCardNum = 64
	// hiAIMaxDeviceNum is the max number of devices in a card
	hiAIMaxDeviceNum = 4

	coreNumLen = 32
	vfgID      = 4294967295 // vfg_id表示指定虚拟设备所属的虚拟分组ID，默认自动分配，默认值为0xFFFFFFFF，转换成10进制为4294967295。
)

// NpuWorker Dcmi worker
type NpuWorker struct {
}

// Initialize dcmi lib init
func (w *NpuWorker) Initialize() error {
	cDlPath := C.CString(string(make([]byte, int32(C.PATH_MAX))))
	defer C.free(unsafe.Pointer(cDlPath))
	if err := C.dcmiInit_dl(cDlPath); err != C.SUCCESS {
		errInfo := fmt.Errorf("dcmi lib load failed, , error code: %d", int32(err))
		return errInfo
	}
	dlPath := C.GoString(cDlPath)
	if _, err := mindxcheckutils.RealFileChecker(dlPath, true, false, mindxcheckutils.DefaultSize); err != nil {
		return err
	}
	if err := C.dcmi_init(); err != C.SUCCESS {
		errInfo := fmt.Errorf("dcmi init failed, , error code: %d", int32(err))
		return errInfo
	}
	return nil
}

// ShutDown shutdown dcmi lib
func (w *NpuWorker) ShutDown() {
	if err := C.dcmiShutDown(); err != C.SUCCESS {
		println(fmt.Errorf("dcmi shut down failed, error code: %d", int32(err)))
	}
}

// GetCardList  list all cards on system
func GetCardList() (int32, []int32, error) {
	var ids [hiAIMaxCardNum]C.int
	var cNum C.int
	if err := C.dcmi_get_card_num_list(&cNum, &ids[0], hiAIMaxCardNum); err != 0 {
		errInfo := fmt.Errorf("get card list failed, error code: %d", int32(err))
		return retError, nil, errInfo
	}
	// checking card's quantity
	if cNum <= 0 || cNum > hiAIMaxCardNum {
		errInfo := fmt.Errorf("get error card quantity: %d", int32(cNum))
		return retError, nil, errInfo
	}
	var cardNum = int32(cNum)
	var cardIDList []int32
	for i := int32(0); i < cardNum; i++ {
		cardID := int32(ids[i])
		if cardID < 0 {
			continue
		}
		cardIDList = append(cardIDList, cardID)
	}
	return cardNum, cardIDList, nil
}

// GetDeviceNumInCard get device number in the npu card
func GetDeviceNumInCard(cardID int32) (int32, error) {
	var deviceNum C.int
	if err := C.dcmi_get_device_num_in_card(C.int(cardID), &deviceNum); err != 0 {
		errInfo := fmt.Errorf("get device count on the card failed, error code: %d", int32(err))
		return retError, errInfo
	}
	if deviceNum <= 0 || deviceNum > hiAIMaxDeviceNum {
		errInfo := fmt.Errorf("the number of chips obtained is invalid, the number is: %d", int32(deviceNum))
		return retError, errInfo
	}
	return int32(deviceNum), nil
}

// GetDeviceLogicID get device logicID
func GetDeviceLogicID(cardID, deviceID int32) (int32, error) {
	var logicID C.int
	if err := C.dcmi_get_device_logic_id(&logicID, C.int(cardID), C.int(deviceID)); err != 0 {
		errInfo := fmt.Errorf("get logicID failed, error code: %d", int32(err))
		return retError, errInfo
	}

	// check whether phyID is too big
	if logicID < 0 || uint32(logicID) > uint32(math.MaxInt8) {
		errInfo := fmt.Errorf("the logicID value is invalid, logicID is: %d", logicID)
		return retError, errInfo
	}
	return int32(logicID), nil
}

// CreateVDevice create virtual device
func (w *NpuWorker) CreateVDevice(cardID, deviceID int32, coreNum string) (int32, error) {
	var createInfo C.struct_dcmi_create_vdev_out
	createInfo.vdev_id = C.uint(math.MaxUint32)
	var deviceCreateStr C.struct_dcmi_create_vdev_res_stru
	deviceCreateStr = C.struct_dcmi_create_vdev_res_stru{
		vdev_id: C.uint(vfgID),
		vfg_id:  C.uint(vfgID),
	}
	deviceCreateStrArr := [coreNumLen]C.char{0}
	for i := 0; i < len(coreNum); i++ {
		if i >= coreNumLen {
			return math.MaxInt32, fmt.Errorf("wrong template")
		}
		deviceCreateStrArr[i] = C.char(coreNum[i])
	}
	deviceCreateStr.template_name = deviceCreateStrArr
	err := C.dcmi_create_vdevice(C.int(cardID), C.int(deviceID), &deviceCreateStr, &createInfo)
	if err != 0 {
		errInfo := fmt.Errorf("create virtual device failed, error code: %d", int32(err))
		return math.MaxInt32, errInfo
	}
	if createInfo.vdev_id > math.MaxInt32 {
		return math.MaxInt32, fmt.Errorf("create virtual device failed, vdeviceId too large")
	}
	return int32(createInfo.vdev_id), nil
}

// DestroyVDevice destroy virtual device
func (w *NpuWorker) DestroyVDevice(cardID, deviceID int32, vDevID int32) error {
	if vDevID < 0 {
		return fmt.Errorf("param error on vDevID")
	}
	if err := C.dcmi_set_destroy_vdevice(C.int(cardID), C.int(deviceID), C.uint(vDevID)); err != 0 {
		errInfo := fmt.Errorf("destroy virtual device failed, error code: %d", int32(err))
		return errInfo
	}
	return nil
}

// FindDevice find device by phyical id
func (w *NpuWorker) FindDevice(visibleDevice int32) (int32, int32, error) {
	var dcmiLogicID C.uint
	if err := C.dcmi_get_device_logicid_from_phyid(C.uint(visibleDevice), &dcmiLogicID); err != 0 {
		return 0, 0, fmt.Errorf("phy id can not be converted to logic id : %v", err)
	}
	if int32(dcmiLogicID) < 0 || int32(dcmiLogicID) >= hiAIMaxCardNum*hiAIMaxDeviceNum {
		return 0, 0, fmt.Errorf("logic id too large")
	}
	targetLogicID := int32(dcmiLogicID)
	_, cardList, err := GetCardList()
	if err != nil {
		return 0, 0, fmt.Errorf("get card list err : %v", err)
	}
	targetDeviceID, targetCardID := int32(math.MaxInt32), int32(math.MaxInt32)
	for _, cardID := range cardList {
		deviceCount, err := GetDeviceNumInCard(cardID)
		if err != nil {
			return 0, 0, fmt.Errorf("cannot get device num in card : %v", err)
		}
		for deviceID := int32(0); deviceID < deviceCount; deviceID++ {
			logicID, err := GetDeviceLogicID(cardID, deviceID)
			if err != nil {
				return 0, 0, fmt.Errorf("cannot get logic id : %v", err)
			}
			if logicID == int32(targetLogicID) {
				targetCardID, targetDeviceID = cardID, deviceID
			}
		}
	}
	return targetDeviceID, targetCardID, nil
}

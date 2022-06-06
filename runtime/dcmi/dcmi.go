// Package dcmi is used to work with Ascend devices
/*
 * Copyright(C) Huawei Technologies Co.,Ltd. 2020-2021. All rights reserved.
 */
package dcmi

// #cgo LDFLAGS: -ldl
// #include "dcmi_interface_api.h"
import "C"
import (
	"fmt"
	"math"
	"unsafe"
)

const (
	// RetError return error when the function failed
	retError = -1

	// dcmiMaxVdevNum is max number of vdevice, value is from driver specification
	dcmiMaxVdevNum = 16

	// maxErrorCodeCount is the max number of error code
	hiAIMaxCardNum = 16
)

// NpuWorker Dcmi worker
type NpuWorker struct {
}

// Initialize dcmi lib init
func (w *NpuWorker) Initialize() error {
	if err := C.dcmiInit_dl(); err != C.SUCCESS {
		errInfo := fmt.Errorf("dcmi lib load failed, , error code: %d", int32(err))
		return errInfo
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
	for i := int32(0); i < cardNum && i < hiAIMaxCardNum; i++ {
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
	if deviceNum <= 0 {
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
	coreTemplate := C.CString(coreNum)
	defer C.free(unsafe.Pointer(coreTemplate))
	err := C.dcmi_create_vdevice(C.int(cardID), C.int(deviceID), C.int(0), coreTemplate, &createInfo)
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
	var logicID C.uint
	if err := C.dcmi_get_device_logicid_from_phyid(C.uint(visibleDevice), &logicID); err != 0 {
		return 0, 0, fmt.Errorf("phy id can not be converted to logic id : %v", err)
	}
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
			if logicID == int32(logicID) {
				targetCardID, targetDeviceID = cardID, deviceID
			}
		}
	}
	return targetDeviceID, targetCardID, nil
}

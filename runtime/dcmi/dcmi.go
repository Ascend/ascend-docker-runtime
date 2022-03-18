// Package dcmi is used to work with Ascend devices
/*
 * Copyright(C) Huawei Technologies Co.,Ltd. 2020-2021. All rights reserved.
 */
package dcmi

// #cgo LDFLAGS: -ldl
/* #include <stddef.h>
  #include <dlfcn.h>
  #include <stdlib.h>
  #include <stdio.h>

  #include "dcmi_interface_api.h"

  void *dcmiHandle;
  #define SO_NOT_FOUND  -99999
  #define FUNCTION_NOT_FOUND  -99998
  #define SUCCESS  0
  #define ERROR_UNKNOWN  -99997
  #define CALL_FUNC(name,...) if(name##_func==NULL){return FUNCTION_NOT_FOUND;}return name##_func(__VA_ARGS__);

  // dcmi
  int (*dcmi_init_func)();
  int dcmi_init(){
	  CALL_FUNC(dcmi_init)
  }

  int (*dcmi_get_card_num_list_func)(int *card_num, int *card_list, int list_length);
  int dcmi_get_card_num_list(int *card_num, int *card_list, int list_length){
	  CALL_FUNC(dcmi_get_card_num_list,card_num,card_list,list_length)
  }

  int (*dcmi_get_device_num_in_card_func)(int card_id, int *device_num);
  int dcmi_get_device_num_in_card(int card_id, int *device_num){
	  CALL_FUNC(dcmi_get_device_num_in_card,card_id,device_num)
  }

  int (*dcmi_get_device_id_in_card_func)(int card_id, int *device_id_max, int *mcu_id, int *cpu_id);
  int dcmi_get_device_id_in_card(int card_id, int *device_id_max, int *mcu_id, int *cpu_id){
	  CALL_FUNC(dcmi_get_device_id_in_card,card_id,device_id_max,mcu_id,cpu_id)
  }

  int (*dcmi_get_device_logic_id_func)(int *device_logic_id, int card_id, int device_id);
  int dcmi_get_device_logic_id(int *device_logic_id, int card_id, int device_id){
	  CALL_FUNC(dcmi_get_device_logic_id,device_logic_id,card_id,device_id)
  }

  int (*dcmi_set_create_vdevice_func)(int card_id, int device_id, struct dcmi_vdev_create_info *info);
  int dcmi_set_create_vdevice(int card_id, int device_id, struct dcmi_vdev_create_info *info){
	  CALL_FUNC(dcmi_set_create_vdevice,card_id,device_id,info)
  }

  int (*dcmi_set_destroy_vdevice_func)(int card_id, int device_id, unsigned int VDevid);
  int dcmi_set_destroy_vdevice(int card_id, int device_id, unsigned int VDevid){
	  CALL_FUNC(dcmi_set_destroy_vdevice,card_id,device_id,VDevid)
  }

  int (*dcmi_get_vdevice_info_func)(int card_id, int device_id, struct dcmi_vdev_info *info);
  int dcmi_get_vdevice_info(int card_id, int device_id, struct dcmi_vdev_info *info){
	  CALL_FUNC(dcmi_get_vdevice_info,card_id,device_id,info)
  }

  int (*dcmi_get_device_health_func)(int card_id, int device_id, unsigned int *health);
  int dcmi_get_device_health(int card_id, int device_id, unsigned int *health){
	  CALL_FUNC(dcmi_get_device_health,card_id,device_id,health)
  }

  int (*dcmi_get_device_chip_info_func)(int card_id, int device_id, struct dcmi_chip_info *chip_info);
  int dcmi_get_device_chip_info(int card_id, int device_id, struct dcmi_chip_info *chip_info){
	  CALL_FUNC(dcmi_get_device_chip_info,card_id,device_id,chip_info)
  }

  int (*dcmi_create_vdevice_func)(int card_id, int device_id, int vdev_id, const char *template_name,
    struct dcmi_create_vdev_out *out);
  int dcmi_create_vdevice(int card_id, int device_id, int vdev_id, const char *template_name,
    struct dcmi_create_vdev_out *out){
	  CALL_FUNC(dcmi_create_vdevice,card_id,device_id,vdev_id,template_name,out)
  }

  // load .so files and functions
  int dcmiInit_dl(void){
	  dcmiHandle = dlopen("libdcmi.so",RTLD_LAZY | RTLD_GLOBAL);
	  if (dcmiHandle == NULL){
		  fprintf (stderr,"%s\n",dlerror());
		  return SO_NOT_FOUND;
	  }

	  dcmi_init_func = dlsym(dcmiHandle,"dcmi_init");

	  dcmi_get_card_num_list_func = dlsym(dcmiHandle,"dcmi_get_card_num_list");

	  dcmi_get_device_num_in_card_func = dlsym(dcmiHandle,"dcmi_get_device_num_in_card");

	  dcmi_get_device_id_in_card_func = dlsym(dcmiHandle,"dcmi_get_device_id_in_card");

	  dcmi_get_device_logic_id_func = dlsym(dcmiHandle,"dcmi_get_device_logic_id");

	  dcmi_set_create_vdevice_func = dlsym(dcmiHandle,"dcmi_set_create_vdevice");

	  dcmi_set_destroy_vdevice_func = dlsym(dcmiHandle,"dcmi_set_destroy_vdevice");

	  dcmi_get_vdevice_info_func = dlsym(dcmiHandle,"dcmi_get_vdevice_info");

	  dcmi_get_device_health_func = dlsym(dcmiHandle,"dcmi_get_device_health");

	  dcmi_get_device_chip_info_func = dlsym(dcmiHandle,"dcmi_get_device_chip_info");

      dcmi_create_vdevice_func = dlsym(dcmiHandle,"dcmi_create_vdevice");

	  return SUCCESS;
  }

  int dcmiShutDown(void){
	  if (dcmiHandle == NULL) {
		  return SUCCESS;
	  }
	  return (dlclose(dcmiHandle) ? ERROR_UNKNOWN : SUCCESS);
  }

  int (*dsmi_get_logicid_from_phyid_func)(unsigned int phyid, unsigned int *logicid);
  int dsmi_get_logicid_from_phyid(unsigned int phyid, unsigned int *logicid){
    CALL_FUNC(dsmi_get_logicid_from_phyid,phyid,logicid)
  }
  void *dsmiHandle;
  int dsmiInit_dl(void){
	dsmiHandle = dlopen("libdrvdsmi_host.so",RTLD_LAZY);
	if (dsmiHandle == NULL) {
		dsmiHandle = dlopen("libdrvdsmi.so",RTLD_LAZY);
	}
	if (dsmiHandle == NULL){
		return SO_NOT_FOUND;
	}

	dsmi_get_logicid_from_phyid_func = dlsym(dsmiHandle,"dsmi_get_logicid_from_phyid");

	return SUCCESS;
}

int dsmiShutDown(void){
	if (dsmiHandle == NULL) {
   	 	return SUCCESS;
  	}
	return (dlclose(dsmiHandle) ? ERROR_UNKNOWN : SUCCESS);
}
*/
import "C"
import (
	"fmt"
	"math"
	"strconv"
	"strings"
	"unsafe"

	"github.com/opencontainers/runtime-spec/specs-go"
)

const (
	// RetError return error when the function failed
	retError = -1

	// dcmiMaxVdevNum is max number of vdevice, value is from driver specification
	dcmiMaxVdevNum = 16

	// maxErrorCodeCount is the max number of error code
	hiAIMaxCardNum = 16
)

// VDeviceInfo vdevice created info
type VDeviceInfo struct {
	CardID    int32
	DeviceID  int32
	VdeviceID int32
}

// InitDcmi dcmi/dsmi lib
func InitDcmi() error {
	if err := C.dcmiInit_dl(); err != C.SUCCESS {
		errInfo := fmt.Errorf("dcmi lib load failed, , error code: %d", int32(err))
		return errInfo
	}
	if err := C.dcmi_init(); err != C.SUCCESS {
		errInfo := fmt.Errorf("dcmi init failed, , error code: %d", int32(err))
		return errInfo
	}
	if err := C.dsmiInit_dl(); err != C.SUCCESS {
		errInfo := fmt.Errorf("dsmi lib load failed, , error code: %d", int32(err))
		return errInfo
	}
	return nil
}

// ShutDownDcmi shutdown dcmi/dsmi lib
func ShutDownDcmi() {
	if err := C.dcmiShutDown(); err != C.SUCCESS {
		println(fmt.Errorf("dcmi shut down failed, error code: %d", int32(err)))
	}
	if err := C.dsmiShutDown(); err != C.SUCCESS {
		println(fmt.Errorf("dsmi shut down failed, error code: %d", int32(err)))
	}
}

func getCardList() (int32, []int32, error) {
	var ids [hiAIMaxCardNum]C.int
	var cNum C.int
	if err := C.dcmi_get_card_num_list(&cNum, &ids[0], hiAIMaxCardNum); err != 0 {
		errInfo := fmt.Errorf("get card list failed, error code: %d", int32(err))
		return retError, nil, errInfo
	}
	// checking card's quantity
	if cNum <= 0 {
		errInfo := fmt.Errorf("get error card quantity: %d", int32(cNum))
		return retError, nil, errInfo
	}
	var cardNum = int32(cNum)
	var i int32
	var cardIDList []int32
	for i = 0; i < cardNum && i < hiAIMaxCardNum; i++ {
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

// SetCreateVDevice create virtual device
func SetCreateVDevice(cardID, deviceID int32, coreNum string) (uint32, error) {
	var createInfo C.struct_dcmi_create_vdev_out
	createInfo.vdev_id = C.uint(math.MaxUint32)
	coreTemplate := C.CString(coreNum)
	defer C.free(unsafe.Pointer(coreTemplate))
	err := C.dcmi_create_vdevice(C.int(cardID), C.int(deviceID), C.int(0), coreTemplate, &createInfo)
	if err != 0 {
		errInfo := fmt.Errorf("create virtual device failed, error code: %d", int32(err))
		return uint32(math.MaxUint32), errInfo
	}
	println("vdevId", createInfo.vdev_id)
	return uint32(createInfo.vdev_id), nil
}

// SetDestroyVDevice destroy virtual device
func SetDestroyVDevice(cardID, deviceID int32, vDevID uint32) error {
	if err := C.dcmi_set_destroy_vdevice(C.int(cardID), C.int(deviceID), C.uint(vDevID)); err != 0 {
		errInfo := fmt.Errorf("destroy virtual device failed, error code: %d", int32(err))
		return errInfo
	}
	return nil
}

// CreateVDevice will create virtual device
func CreateVDevice(spec *specs.Spec) (VDeviceInfo, error) {
	visibleDevice, splitDevice, err := extractVpuParam(spec)
	invalidVDevice := VDeviceInfo{CardID: -1, DeviceID: -1, VdeviceID: -1}
	if err != nil || visibleDevice == -1 {
		return invalidVDevice, err
	}
	if err := InitDcmi(); err != nil {
		return invalidVDevice, fmt.Errorf("cannot init dcmi : %v", err)
	}
	defer ShutDownDcmi()
	var dsmiLogicID C.uint
	if err := C.dsmi_get_logicid_from_phyid(C.uint(visibleDevice), &dsmiLogicID); err != 0 {
		return invalidVDevice, fmt.Errorf("phy id can not be converted to logic id : %v", err)
	}
	_, cardList, err := getCardList()
	targetDeviceID, targetCardID := int32(math.MaxInt32), int32(math.MaxInt32)
	for _, cardID := range cardList {
		deviceCount, err := GetDeviceNumInCard(cardID)
		if err != nil {
			return invalidVDevice, fmt.Errorf("cannot get device num in card : %v", err)
		}
		for deviceID := int32(0); deviceID < deviceCount; deviceID++ {
			logicID, err := GetDeviceLogicID(cardID, deviceID)
			println(cardID, deviceID, logicID, dsmiLogicID)
			if err != nil {
				return invalidVDevice, fmt.Errorf("cannot get logic id : %v", err)
			}
			if logicID == int32(dsmiLogicID) {
				targetCardID, targetDeviceID = cardID, deviceID
			}
		}
	}

	vdeviceID, err := SetCreateVDevice(targetCardID, targetDeviceID, splitDevice)
	if err != nil || int(vdeviceID) < 0 {
		return invalidVDevice, fmt.Errorf("cannot create vd or vdevice is wrong: %v %v", vdeviceID, err)
	}
	fmt.Printf("%v", VDeviceInfo{CardID: targetCardID, DeviceID: targetDeviceID, VdeviceID: int32(vdeviceID)})
	return VDeviceInfo{CardID: targetCardID, DeviceID: targetDeviceID, VdeviceID: int32(vdeviceID)}, nil
}

func extractVpuParam(spec *specs.Spec) (int32, string, error) {
	visibleDevice, splitDevice, needSplit, visibleDeviceLine := int32(-1), "", false, ""
	allowSplit := map[string]string{
		"1C": "vir01", "2C": "vir02", "4C": "vir04", "8C": "vir08", "16C": "vir16",
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
		if strings.TrimSpace(words[0]) == "ASCEND_VISIBLE_VPU_DEVICES" {
			if split := allowSplit[words[1]]; split != "" {
				splitDevice = split
				needSplit = true
			} else {
				return -1, "", fmt.Errorf("cannot parse param : %v", words[1])
			}
		}
	}
	if needSplit {
		if cardID, err := strconv.Atoi(visibleDeviceLine); err == nil {
			visibleDevice = int32(cardID)
		} else {
			return -1, "", fmt.Errorf("cannot parse param : %v %v", err, visibleDeviceLine)
		}
	} else {
		return -1, "", nil
	}
	return visibleDevice, splitDevice, nil
}

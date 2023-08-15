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
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"syscall"

	"github.com/containerd/containerd/oci"
	"github.com/opencontainers/runtime-spec/specs-go"
	"huawei.com/npu-exporter/v5/common-utils/hwlog"
	"k8s.io/api/core/v1"

	"main/dcmi"
	"mindxcheckutils"
)

const (
	runLogPath          = "/var/log/ascend-docker-runtime/runtime-run.log"
	operateLogPath      = "/var/log/ascend-docker-runtime/runtime-operate.log"
	hookDefaultFilePath = "/usr/local/bin/ascend-docker-hook"

	maxCommandLength = 65535
	hookCli          = "ascend-docker-hook"
	destroyHookCli   = "ascend-docker-destroy"
	dockerRuncFile   = "docker-runc"
	runcFile         = "runc"
	envLength        = 2
	kvPairSize       = 2
	borderNum        = 2

	// ENV for device-plugin to identify ascend-docker-runtime
	useAscendDocker      = "ASCEND_DOCKER_RUNTIME=True"
	devicePlugin         = "ascend-device-plugin"
	ascendVisibleDevices = "ASCEND_VISIBLE_DEVICES"
	ascendRuntimeOptions = "ASCEND_RUNTIME_OPTIONS"
)

var (
	hookCliPath     = hookCli
	hookDefaultFile = hookDefaultFilePath
	dockerRuncName  = dockerRuncFile
	runcName        = runcFile

	notMatchError = errors.New("container not match pod or pod not has huawei.com/Ascend910 annotation")
)

const (
	// Atlas200ISoc Product name
	Atlas200ISoc = "Atlas 200I SoC A1"
	// Atlas200 Product name
	Atlas200 = "Atlas 200 Model 3000"
	// Ascend310 ascend 310 chip
	Ascend310 = "Ascend310"
	// Ascend310P ascend 310P chip
	Ascend310P = "Ascend310P"
	// Ascend310B ascend 310B chip
	Ascend310B = "Ascend310B"
	// Ascend910 ascend 910 chip
	Ascend910 = "Ascend910"

	devicePath           = "/dev/"
	davinciName          = "davinci"
	virtualDavinciName   = "vdavinci"
	davinciManager       = "davinci_manager"
	davinciManagerDocker = "davinci_manager_docker"
	notRenameDeviceType  = ""
	devmmSvm             = "devmm_svm"
	hisiHdc              = "hisi_hdc"
	svm0                 = "svm0"
	tsAisle              = "ts_aisle"
	upgrade              = "upgrade"
	sys                  = "sys"
	vdec                 = "vdec"
	vpc                  = "vpc"
	pngd                 = "pngd"
	venc                 = "venc"
	dvppCmdList          = "dvpp_cmdlist"
	logDrv               = "log_drv"
	acodec               = "acodec"
	ai                   = "ai"
	ao                   = "ao"
	vo                   = "vo"
	hdmi                 = "hdmi"
)

type args struct {
	bundleDirPath string
	cmd           string
}

// GetDeviceTypeByChipName get device type by chipName
func GetDeviceTypeByChipName(chipName string) string {
	if strings.Contains(chipName, "310B") {
		return Ascend310B
	}
	if strings.Contains(chipName, "310P") {
		return Ascend310P
	}
	if strings.Contains(chipName, "310") {
		return Ascend310
	}
	if strings.Contains(chipName, "910") {
		return Ascend910
	}
	return ""
}

func getArgs() (*args, error) {
	args := &args{}

	for i, param := range os.Args {
		if param == "--bundle" || param == "-b" {
			if len(os.Args)-i <= 1 {
				return nil, fmt.Errorf("bundle option needs an argument")
			}
			args.bundleDirPath = os.Args[i+1]
		} else if param == "create" {
			args.cmd = param
		}
	}

	return args, nil
}

func initLogModule(ctx context.Context) error {
	const backups = 2
	const logMaxAge = 365
	runLogConfig := hwlog.LogConfig{
		LogFileName: runLogPath,
		LogLevel:    0,
		MaxBackups:  backups,
		MaxAge:      logMaxAge,
		OnlyToFile:  true,
		FileMaxSize: 2,
	}
	if err := hwlog.InitRunLogger(&runLogConfig, ctx); err != nil {
		fmt.Printf("hwlog init failed, error is %v", err)
		return err
	}
	operateLogConfig := hwlog.LogConfig{
		LogFileName: operateLogPath,
		LogLevel:    0,
		MaxBackups:  backups,
		MaxAge:      logMaxAge,
		OnlyToFile:  true,
		FileMaxSize: 2,
	}
	if err := hwlog.InitOperateLogger(&operateLogConfig, ctx); err != nil {
		fmt.Printf("hwlog init failed, error is %v", err)
		return err
	}
	return nil
}

var execRunc = func() error {
	tempRuncPath, err := exec.LookPath(dockerRuncName)
	if err != nil {
		tempRuncPath, err = exec.LookPath(runcName)
		if err != nil {
			return fmt.Errorf("failed to find the path of runc: %v", err)
		}
	}
	runcPath, err := filepath.EvalSymlinks(tempRuncPath)
	if err != nil {
		return fmt.Errorf("failed to find realpath of runc %v", err)
	}
	if _, err := mindxcheckutils.RealFileChecker(runcPath, true, false, mindxcheckutils.DefaultSize); err != nil {
		return err
	}

	hwlog.OpLog.Infof("ascend docker runtime success, will start runc")
	if err := mindxcheckutils.ChangeRuntimeLogMode("runtime-run-", "runtime-operate-"); err != nil {
		return err
	}
	if err = syscall.Exec(runcPath, append([]string{runcPath}, os.Args[1:]...), os.Environ()); err != nil {
		return fmt.Errorf("failed to exec runc: %v", err)
	}

	return nil
}

func addEnvToDevicePlugin(spec *specs.Spec) {
	if spec.Process.Env == nil {
		return
	}

	for _, line := range spec.Process.Env {
		words := strings.Split(line, "=")
		if len(words) == envLength && strings.TrimSpace(words[0]) == "HOSTNAME" &&
			strings.Contains(words[1], devicePlugin) {
			spec.Process.Env = append(spec.Process.Env, useAscendDocker)
			break
		}
	}
}

func addHook(spec *specs.Spec) error {
	currentExecPath, err := os.Executable()
	if err != nil {
		return fmt.Errorf("cannot get the path of ascend-docker-runtime: %v", err)
	}

	hookCliPath = path.Join(path.Dir(currentExecPath), hookCli)
	if _, err := mindxcheckutils.RealFileChecker(hookCliPath, true, false, mindxcheckutils.DefaultSize); err != nil {
		return err
	}
	if _, err = os.Stat(hookCliPath); err != nil {
		return fmt.Errorf("cannot find ascend-docker-hook executable file at %s: %v", hookCliPath, err)
	}

	if spec.Hooks == nil {
		spec.Hooks = &specs.Hooks{}
	}

	needUpdate := true
	if len(spec.Hooks.Prestart) > maxCommandLength {
		return fmt.Errorf("too many items in Prestart ")
	}
	for _, hook := range spec.Hooks.Prestart {
		if strings.Contains(hook.Path, hookCli) {
			needUpdate = false
			break
		}
	}
	if needUpdate {
		spec.Hooks.Prestart = append(spec.Hooks.Prestart, specs.Hook{
			Path: hookCliPath,
			Args: []string{hookCliPath},
		})
	}

	if len(spec.Process.Env) > maxCommandLength {
		return fmt.Errorf("too many items in Env ")
	}
	for _, line := range spec.Process.Env {
		words := strings.Split(line, "=")
		if len(words) == envLength && strings.TrimSpace(words[0]) == "ASCEND_RUNTIME_OPTIONS" &&
			strings.Contains(words[1], "VIRTUAL") {
			return nil
		}
	}

	vdevice, err := dcmi.CreateVDevice(&dcmi.NpuWorker{}, spec)
	if err != nil {
		return err
	}
	hwlog.RunLog.Infof("vnpu split done: vdevice: %v", vdevice.VdeviceID)

	if vdevice.VdeviceID != -1 {
		updateEnvAndPostHook(spec, vdevice)
	}

	return nil
}

func removeDuplication(devices []int) []int {
	list := make([]int, 0, len(devices))
	prev := -1

	for _, device := range devices {
		if device == prev {
			continue
		}

		list = append(list, device)
		prev = device
	}

	return list
}

func parseAnnotationDevices(annotationDevices string) ([]int, error) {
	devices := make([]int, 0)

	for _, d := range strings.Split(annotationDevices, ",") {
		borders := strings.Split(d, Ascend910+"-")
		if len(borders) != borderNum || borders[0] != "" {
			return nil, fmt.Errorf("invalid device range: %s", d)
		}
		deviceID, err := strconv.Atoi(borders[1])
		if err != nil {
			return nil, fmt.Errorf("invalid device ID: %s", d)
		}

		devices = append(devices, deviceID)
	}

	sort.Slice(devices, func(i, j int) bool { return i < j })
	return removeDuplication(devices), nil
}

func parseDevices(visibleDevices string) ([]int, error) {
	devices := make([]int, 0)
	const maxDevice = 128

	for _, d := range strings.Split(visibleDevices, ",") {
		d = strings.TrimSpace(d)
		if strings.Contains(d, "-") {
			borders := strings.Split(d, "-")
			if len(borders) != borderNum {
				return nil, fmt.Errorf("invalid device range: %s", d)
			}

			borders[0] = strings.TrimSpace(borders[0])
			borders[1] = strings.TrimSpace(borders[1])

			left, err := strconv.Atoi(borders[0])
			if err != nil || left < 0 {
				return nil, fmt.Errorf("invalid left boarder range parameter: %s", borders[0])
			}

			right, err := strconv.Atoi(borders[1])
			if err != nil || right > maxDevice {
				return nil, fmt.Errorf("invalid right boarder range parameter: %s", borders[1])
			}

			if left > right {
				return nil, fmt.Errorf("left boarder (%d) should not be larger than the right one(%d)", left, right)
			}

			for n := left; n <= right; n++ {
				devices = append(devices, n)
			}
		} else {
			n, err := strconv.Atoi(d)
			if err != nil {
				return nil, fmt.Errorf("invalid single device parameter: %s", d)
			}

			devices = append(devices, n)
		}
	}

	sort.Slice(devices, func(i, j int) bool { return i < j })
	return removeDuplication(devices), nil
}

func getValueByKey(data []string, name string) string {
	for _, envLine := range data {
		words := strings.SplitN(envLine, "=", kvPairSize)
		if len(words) != kvPairSize {
			hwlog.RunLog.Errorf("environment error")
			return ""
		}

		if words[0] == name {
			return words[1]
		}
	}

	return ""
}

func addDeviceToSpec(spec *specs.Spec, dPath string, deviceType string) error {
	device, err := oci.DeviceFromPath(dPath)
	if err != nil {
		return fmt.Errorf("failed to get %s info : %#v", dPath, err)
	}

	switch deviceType {
	case virtualDavinciName:
		vDeviceNumber := regexp.MustCompile("[0-9]+").FindAllString(dPath, -1)
		if len(vDeviceNumber) != 1 {
			return fmt.Errorf("invalid vdavinci path: %s", dPath)
		}
		device.Path = devicePath + davinciName + vDeviceNumber[0]
	case davinciManagerDocker:
		device.Path = devicePath + davinciManager
	default:
		// do nothing
	}

	spec.Linux.Devices = append(spec.Linux.Devices, *device)
	newDeviceCgroup := specs.LinuxDeviceCgroup{
		Allow:  true,
		Type:   device.Type,
		Major:  &device.Major,
		Minor:  &device.Minor,
		Access: "rwm",
	}
	spec.Linux.Resources.Devices = append(spec.Linux.Resources.Devices, newDeviceCgroup)
	return nil
}

func addAscend310BManagerDevice(spec *specs.Spec) error {
	var Ascend310BManageDevices = []string{
		svm0,
		tsAisle,
		upgrade,
		sys,
		vdec,
		vpc,
		pngd,
		venc,
		dvppCmdList,
		logDrv,
		acodec,
		ai,
		ao,
		vo,
		hdmi,
	}

	for _, device := range Ascend310BManageDevices {
		dPath := devicePath + device
		if err := addDeviceToSpec(spec, dPath, notRenameDeviceType); err != nil {
			hwlog.RunLog.Warnf("failed to add %s to spec : %#v", dPath, err)
		}
	}

	davinciManagerPath := devicePath + davinciManagerDocker
	if _, err := os.Stat(davinciManagerPath); err != nil {
		hwlog.RunLog.Warnf("failed to get davinci manager docker, err: %#v", err)
		davinciManagerPath = devicePath + davinciManager
		if _, err := os.Stat(davinciManagerPath); err != nil {
			return fmt.Errorf("failed to get davinci manager, err: %#v", err)
		}
	}
	return addDeviceToSpec(spec, davinciManagerPath, davinciManagerDocker)
}

func addCommonManagerDevice(spec *specs.Spec) error {
	var commonManagerDevices = []string{
		devmmSvm,
		hisiHdc,
	}

	for _, device := range commonManagerDevices {
		dPath := devicePath + device
		if err := addDeviceToSpec(spec, dPath, notRenameDeviceType); err != nil {
			return fmt.Errorf("failed to add common manage device to spec : %#v", err)
		}
	}

	return nil
}

func addManagerDevice(spec *specs.Spec) error {
	chipName, err := dcmi.GetChipName()
	if err != nil {
		return fmt.Errorf("get chip name error: %#v", err)
	}
	devType := GetDeviceTypeByChipName(chipName)
	hwlog.RunLog.Infof("device type is: %s", devType)
	if devType == Ascend310B {
		return addAscend310BManagerDevice(spec)
	}

	if err := addDeviceToSpec(spec, devicePath+davinciManager, notRenameDeviceType); err != nil {
		return fmt.Errorf("add davinci_manager to spec error: %#v", err)
	}

	productType, err := dcmi.GetProductType(&dcmi.NpuWorker{})
	if err != nil {
		return fmt.Errorf("parse product type error: %#v", err)
	}
	hwlog.RunLog.Infof("product type is %s", productType)

	switch productType {
	// do nothing
	case Atlas200ISoc, Atlas200:
	default:
		if err = addCommonManagerDevice(spec); err != nil {
			return fmt.Errorf("add common manage device error: %#v", err)
		}
	}

	return nil
}

func addDevice(spec *specs.Spec) error {
	// 获取对应pod annotation中的设备信息
	annotationDevices, err := getDeviceFromPod(spec)
	if err != nil && err != notMatchError {
		hwlog.RunLog.Errorf("getDeviceFromPod failed: %#v", err)
		//return fmt.Errorf("failed to get pod device: %#v", err)
	}

	// 如果没有匹配到pod或annotation，则通过环境变量挂载设备
	if annotationDevices == "" {
		hwlog.RunLog.Info("add devices from env variable")
		if err = addDeviceFromEnv(spec); err != nil {
			return fmt.Errorf("failed to add device to env: %#v", err)
		}
		return nil
	}

	// 如果对应pod annotation中的设备信息存在，则用这个信息挂载设备
	devices, err := parseAnnotationDevices(annotationDevices)
	if err != nil {
		return fmt.Errorf("failed to parse device: %#v", err)
	}
	hwlog.RunLog.Infof("annotation devices is: %#v", devices)
	deviceName := davinciName
	for _, deviceId := range devices {
		dPath := devicePath + deviceName + strconv.Itoa(deviceId)
		if err = addDeviceToSpec(spec, dPath, deviceName); err != nil {
			return fmt.Errorf("failed to add davinci device to spec: %#v", err)
		}
	}

	if err = addManagerDevice(spec); err != nil {
		return fmt.Errorf("failed to add Manager device to spec: %#v", err)
	}

	return nil
}

func addDeviceFromEnv(spec *specs.Spec) error {
	visibleDevices := getValueByKey(spec.Process.Env, ascendVisibleDevices)
	if visibleDevices == "" {
		return nil
	}

	devices, err := parseDevices(visibleDevices)
	if err != nil {
		return fmt.Errorf("failed to parse device : %#v", err)
	}
	hwlog.RunLog.Infof("devices is: %#v", devices)
	deviceName := davinciName
	virtual := getValueByKey(spec.Process.Env, ascendRuntimeOptions)
	if strings.Contains(virtual, "VIRTUAL") {
		deviceName = virtualDavinciName
	}
	for _, deviceId := range devices {
		dPath := devicePath + deviceName + strconv.Itoa(deviceId)
		if err = addDeviceToSpec(spec, dPath, deviceName); err != nil {
			return fmt.Errorf("failed to add davinci device to spec: %v", err)
		}
	}

	if err = addManagerDevice(spec); err != nil {
		return fmt.Errorf("failed to add Manager device to spec: %v", err)
	}

	return nil
}

func updateEnvAndPostHook(spec *specs.Spec, vdevice dcmi.VDeviceInfo) {
	newEnv := make([]string, 0)
	needAddVirtualFlag := true
	for _, line := range spec.Process.Env {
		words := strings.Split(line, "=")
		if len(words) == envLength && strings.TrimSpace(words[0]) == ascendVisibleDevices {
			newEnv = append(newEnv, fmt.Sprintf("ASCEND_VISIBLE_DEVICES=%d", vdevice.VdeviceID))
			continue
		}
		if len(words) == envLength && strings.TrimSpace(words[0]) == ascendRuntimeOptions {
			needAddVirtualFlag = false
			if strings.Contains(words[1], "VIRTUAL") {
				newEnv = append(newEnv, line)
				continue
			} else {
				newEnv = append(newEnv, strings.TrimSpace(line)+",VIRTUAL")
				continue
			}
		}
		newEnv = append(newEnv, line)
	}
	if needAddVirtualFlag {
		newEnv = append(newEnv, fmt.Sprintf("ASCEND_RUNTIME_OPTIONS=VIRTUAL"))
	}
	spec.Process.Env = newEnv
	if currentExecPath, err := os.Executable(); err == nil {
		postHookCliPath := path.Join(path.Dir(currentExecPath), destroyHookCli)
		spec.Hooks.Poststop = append(spec.Hooks.Poststop, specs.Hook{
			Path: postHookCliPath,
			Args: []string{postHookCliPath, fmt.Sprintf("%d", vdevice.CardID), fmt.Sprintf("%d", vdevice.DeviceID),
				fmt.Sprintf("%d", vdevice.VdeviceID)},
		})
	}
}

func modifySpecFile(path string) error {
	stat, err := os.Stat(path)
	if err != nil {
		return fmt.Errorf("spec file doesnt exist %s: %v", path, err)
	}
	if _, err = mindxcheckutils.RealFileChecker(path, true, true, mindxcheckutils.DefaultSize); err != nil {
		return err
	}

	jsonFile, err := os.OpenFile(path, os.O_RDWR, stat.Mode())
	if err != nil {
		return fmt.Errorf("cannot open oci spec file %s: %v", path, err)
	}

	defer jsonFile.Close()

	jsonContent, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		return fmt.Errorf("failed to read oci spec file %s: %v", path, err)
	}

	if err = jsonFile.Truncate(0); err != nil {
		return fmt.Errorf("failed to truncate: %v", err)
	}
	if _, err = jsonFile.Seek(0, 0); err != nil {
		return fmt.Errorf("failed to seek: %v", err)
	}

	var spec specs.Spec
	if err = json.Unmarshal(jsonContent, &spec); err != nil {
		return fmt.Errorf("failed to unmarshal oci spec file %s: %v", path, err)
	}

	if err = addHook(&spec); err != nil {
		return fmt.Errorf("failed to inject hook: %v", err)
	}

	if err = addDevice(&spec); err != nil {
		return fmt.Errorf("failed to add device to env: %v", err)
	}

	addEnvToDevicePlugin(&spec)

	jsonOutput, err := json.Marshal(spec)
	if err != nil {
		return fmt.Errorf("failed to marshal OCI spec file: %v", err)
	}

	if _, err = jsonFile.WriteAt(jsonOutput, 0); err != nil {
		return fmt.Errorf("failed to write OCI spec file: %v", err)
	}

	return nil
}

func doProcess() error {
	args, err := getArgs()
	if err != nil {
		return fmt.Errorf("failed to get args: %v", err)
	}

	if args.cmd != "create" {
		return execRunc()
	}

	if args.bundleDirPath == "" {
		args.bundleDirPath, err = os.Getwd()
		if err != nil {
			return fmt.Errorf("failed to get current working dir: %v", err)
		}
	}

	specFilePath := args.bundleDirPath + "/config.json"

	if err = modifySpecFile(specFilePath); err != nil {
		return fmt.Errorf("failed to modify spec file %s: %v", specFilePath, err)
	}

	return execRunc()
}

func main() {
	defer func() {
		if err := recover(); err != nil {
			log.Fatal(err)
		}
	}()
	ctx, _ := context.WithCancel(context.Background())
	if err := initLogModule(ctx); err != nil {
		log.Fatal(err)
	}
	logPrefixWords, err := mindxcheckutils.GetLogPrefix()
	if err != nil {
		log.Fatal(err)
	}
	defer func() {
		if err = mindxcheckutils.ChangeRuntimeLogMode("runtime-run-", "runtime-operate-"); err != nil {
			fmt.Println("defer changeFileMode function failed")
		}
	}()
	hwlog.RunLog.Infof("ascend docker runtime starting")
	hwlog.OpLog.Infof("%v ascend docker runtime starting, try to setup container", logPrefixWords)
	if !mindxcheckutils.StringChecker(strings.Join(os.Args, " "), 0,
		maxCommandLength, mindxcheckutils.DefaultWhiteList+" ") {
		hwlog.RunLog.Errorf("%v ascend docker runtime args check failed", logPrefixWords)
		hwlog.OpLog.Errorf("%v ascend docker runtime args check failed", logPrefixWords)
		log.Fatal("command error")
	}
	if err = doProcess(); err != nil {
		hwlog.RunLog.Errorf("%v docker runtime failed: %v", logPrefixWords, err)
		hwlog.OpLog.Errorf("%v docker runtime failed: %v", logPrefixWords, err)
		log.Fatal(err)
	}
}

func getDeviceFromPod(spec *specs.Spec) (string, error) {
	// 只有apiserver会访问kubelet的https api接口，所以使用apiserver的客户端证书；证书需要从master节点拷贝到worker节点
	certFile := "/etc/kubernetes/pki/apiserver-kubelet-client.crt"
	keyFile := "/etc/kubernetes/pki/apiserver-kubelet-client.key"
	kubeletUrl := "https://127.0.0.1:10250/"
	podsUrlPath := "pods"
	npu910CardName := "huawei.com/Ascend910"

	cert, err := tls.LoadX509KeyPair(certFile, keyFile)
	if err != nil {
		hwlog.RunLog.Errorf("LoadX509KeyPair failed: %#v", err)
		return "", err
	}

	// 构造带客户端证书的http客户端
	client := &http.Client{
		Transport: &http.Transport{
			Proxy: nil, // 禁用代理
			TLSClientConfig: &tls.Config{
				Certificates:       []tls.Certificate{cert},
				InsecureSkipVerify: true, // kubelet 是自签名ca证书，apiserver也未校验kubelet服务端证书，所以这里不校验
			},
		},
	}

	// 向kubelet服务端请求获取pod list
	resp, err := client.Get(kubeletUrl + podsUrlPath)
	if err != nil {
		hwlog.RunLog.Errorf("http get failed: %#v", err)
		return "", err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		hwlog.RunLog.Errorf("ReadAll resp.Body failed: %#v", err)
		return "", err
	}
	hwlog.RunLog.Infof("get pod list success, resp.Status: %#v", resp.Status)

	// 遍历pod list，找到此容器
	var podList v1.PodList
	if err := json.Unmarshal(body, &podList); err != nil {
		hwlog.RunLog.Errorf("unmarshal body failed: %#v", err)
		return "", err
	}

	for _, pod := range podList.Items {
		if pod.ObjectMeta.Name == spec.Hostname {
			if value, ok := pod.ObjectMeta.Annotations[npu910CardName]; ok {
				return value, nil
			}
			break
		}
	}
	return "", notMatchError
}

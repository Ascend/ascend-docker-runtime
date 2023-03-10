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
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"syscall"

	"github.com/containerd/containerd/oci"
	"github.com/opencontainers/runtime-spec/specs-go"
	"huawei.com/npu-exporter/v3/common-utils/hwlog"

	"main/dcmi"
	"mindxcheckutils"
)

const (
	runLogPath          = "/var/log/ascend-docker-runtime/runtime-run.log"
	operateLogPath      = "/var/log/ascend-docker-runtime/runtime-operate.log"
	hookDefaultFilePath = "/usr/local/bin/ascend-docker-hook"
	devicePath          = "/dev/"
	davinciName         = "davinci"
	davinciManager      = "davinci_manager"
	devmmSvm            = "devmm_svm"
	hisiHdc             = "hisi_hdc"
	maxCommandLength    = 65535
	hookCli             = "ascend-docker-hook"
	destroyHookCli      = "ascend-docker-destroy"
	dockerRuncFile      = "docker-runc"
	runcFile            = "runc"
	envLength           = 2
	kvPairSize          = 2
	borderNum           = 2
	vdeviceIdlen        = 3

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
)

type args struct {
	bundleDirPath string
	cmd           string
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

func addDeviceToSpec(spec *specs.Spec, dPath string, vdevice bool) error {
	device, err := oci.DeviceFromPath(dPath)
	if err != nil {
		return fmt.Errorf("failed to get device info : %#v", err)
	}

	lenPath := len(dPath)
	if vdevice {
		vPath := devicePath + davinciName + dPath[lenPath-vdeviceIdlen:]
		device.Path = vPath
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

func addManagerDevice(spec *specs.Spec) error {
	managerPath := devicePath + davinciManager
	if err := addDeviceToSpec(spec, managerPath, false); err != nil {
		return fmt.Errorf("failed to add manager device to spec : %#v", err)
	}

	svmPath := devicePath + devmmSvm
	if _, err := os.Stat(svmPath); err == nil {
		if err = addDeviceToSpec(spec, svmPath, false); err != nil {
			return fmt.Errorf("failed to add devmm_svm to spec : %#v", err)
		}
	}

	hdcPath := devicePath + hisiHdc
	if _, err := os.Stat(hdcPath); err == nil {
		if err = addDeviceToSpec(spec, hdcPath, false); err != nil {
			return fmt.Errorf("failed to add hisi_hdc device to spec : %#v", err)
		}
	}

	return nil
}

func addDevice(spec *specs.Spec) error {
	visibleDevices := getValueByKey(spec.Process.Env, ascendVisibleDevices)
	if visibleDevices == "" {
		return nil
	}
	hwlog.RunLog.Infof("getValueByKey visibleDevices: %#v", visibleDevices)

	devices, err := parseDevices(visibleDevices)
	if err != nil {
		return fmt.Errorf("failed to parse device : %#v", err)
	}
	hwlog.RunLog.Infof("devices is: %#v", devices)
	devciename := davinciName
	vdevice := false
	virtual := getValueByKey(spec.Process.Env, ascendRuntimeOptions)
	if strings.Contains(virtual, "VIRTUAL") {
		devciename = "v" + devciename
		vdevice = true
	}
	for _, deviceId := range devices {
		dPath := devicePath + devciename + strconv.Itoa(deviceId)
		if err = addDeviceToSpec(spec, dPath, vdevice); err != nil {
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

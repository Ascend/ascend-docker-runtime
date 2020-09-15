/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ascend-docker-hook工具，配置容器挂载Ascend NPU设备
 */
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path"
	"sort"
	"strconv"
	"strings"
	"syscall"

	"github.com/opencontainers/runtime-spec/specs-go"
)

const (
	loggingPrefix          = "ascend-docker-hook"
	ascendVisibleDevices   = "ASCEND_VISIBLE_DEVICES"
	ascendRuntimeOptions   = "ASCEND_RUNTIME_OPTIONS"
	ascendDockerCli        = "ascend-docker-cli"
	defaultAscendDockerCli = "/usr/local/bin/ascend-docker-cli"

	borderNum  = 2
	kvPairSize = 2
)

var (
	containerConfigInputStream = os.Stdin
	doExec                     = syscall.Exec
	ascendDockerCliName        = ascendDockerCli
	defaultAscendDockerCliName = defaultAscendDockerCli
)

var validRuntimeOptions = [...]string {
	"NODRV",
	"VERBOSE",
}

type containerConfig struct {
	Pid    int
	Rootfs string
	Env    []string
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
			if err != nil {
				return nil, fmt.Errorf("invalid left boarder range parameter: %s", borders[0])
			}

			right, err := strconv.Atoi(borders[1])
			if err != nil {
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

func isRuntimeOptionValid(option string) bool {
	for _, validOption := range validRuntimeOptions {
		if option == validOption {
			return true
		}
	}

	return false
}

func parseRuntimeOptions(runtimeOptions string) ([]string, error) {
	parsedOptions := make([]string, 0)

	if runtimeOptions == "" {
		return parsedOptions, nil
	}

	for _, option := range strings.Split(runtimeOptions, ",") {
		option = strings.TrimSpace(option)
		if !isRuntimeOptionValid(option) {
			return nil, fmt.Errorf("invalid runtime option %s", option)
		}

		parsedOptions = append(parsedOptions, option)
	}

	return parsedOptions, nil
}

func parseOciSpecFile(file string) (*specs.Spec, error) {
	f, err := os.Open(file)
	if err != nil {
		return nil, fmt.Errorf("failed to open the OCI config file: %s", file)
	}
	defer f.Close()

	spec := new(specs.Spec)
	if err := json.NewDecoder(f).Decode(spec); err != nil {
		return nil, fmt.Errorf("failed to parse OCI config file: %s, caused by: %w", file, err)
	}

	if spec.Process == nil {
		return nil, fmt.Errorf("invalid OCI spec for empty process")
	}

	if spec.Root == nil {
		return nil, fmt.Errorf("invalid OCI spec for empty root")
	}

	return spec, nil
}

var getContainerConfig = func () (*containerConfig, error) {
	state := new(specs.State)
	decoder := json.NewDecoder(containerConfigInputStream)

	if err := decoder.Decode(state); err != nil {
		return nil, fmt.Errorf("failed to parse the container's state")
	}

	configPath := path.Join(state.Bundle, "config.json")
	ociSpec, err := parseOciSpecFile(configPath)
	if err != nil {
		return nil, fmt.Errorf("failed to parse OCI spec: %w", err)
	}

	ret := &containerConfig{
		Pid:    state.Pid,
		Rootfs: ociSpec.Root.Path,
		Env:    ociSpec.Process.Env,
	}

	return ret, nil
}

func getValueByKey(data []string, key string) string {
	for _, s := range data {
		p := strings.SplitN(s, "=", 2)
		if len(p) != kvPairSize {
			log.Panicln("environment error")
		}

		if p[0] == key {
			return p[1]
		}
	}

	return ""
}

func doPrestartHook() error {
	containerConfig, err := getContainerConfig()
	if err != nil {
		return fmt.Errorf("failed to get container config: %w", err)
	}

	visibleDevices := getValueByKey(containerConfig.Env, ascendVisibleDevices)
	if visibleDevices == "" {
		return nil
	}

	devices, err := parseDevices(visibleDevices)
	if err != nil {
		return fmt.Errorf("failed to parse device setting: %w", err)
	}

	parsedOptions, err := parseRuntimeOptions(getValueByKey(containerConfig.Env, ascendRuntimeOptions))
	if err != nil {
		return fmt.Errorf("failed to parse runtime options: %w", err)
	}

	currentExecPath, err := os.Executable()
	if err != nil {
		return fmt.Errorf("cannot get the path of ascend-docker-hook: %w", err)
	}

	cliPath := path.Join(path.Dir(currentExecPath), ascendDockerCliName)
	if _, err = os.Stat(cliPath); err != nil {
		return fmt.Errorf("cannot find ascend-docker-cli executable file at %s: %w", cliPath, err)
	}

	args := append([]string{cliPath},
		"--devices", strings.Trim(strings.Join(strings.Fields(fmt.Sprint(devices)), ","), "[]"),
		"--pid", fmt.Sprintf("%d", containerConfig.Pid),
		"--rootfs", containerConfig.Rootfs)

	if len(parsedOptions) > 0 {
		args = append(args, "--options", strings.Join(parsedOptions, ","))
	}

	if err := doExec(cliPath, args, os.Environ()); err != nil {
		return fmt.Errorf("failed to exec ascend-docker-cli %v: %w", args, err)
	}

	return nil
}

func main() {
	log.SetPrefix(loggingPrefix)
	flag.Parse()

	if err := doPrestartHook(); err != nil {
		log.Fatal(err)
	}
}

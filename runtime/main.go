package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"strings"
	"syscall"

	"github.com/opencontainers/runtime-spec/specs-go"
)

const (
	loggingPrefix       = "ascend-docker-runtime"
	hookCli             = "ascend-docker-hook"
	hookDefaultFilePath = "/usr/local/bin/ascend-docker-hook"
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
				return nil, fmt.Errorf("bundle option needs an argument\n")
			}
			args.bundleDirPath = os.Args[i+1]
		} else if param == "create" {
			args.cmd = param
		}
	}

	return args, nil
}

func execRunc() error {
	runcPath, err := exec.LookPath("docker-runc")
	if err != nil {
		runcPath, err = exec.LookPath("runc")
		if err != nil {
			return fmt.Errorf("failed to find the path of runc: %w\n", err)
		}
	}

	if err = syscall.Exec(runcPath, append([]string{runcPath}, os.Args[1:]...), os.Environ()); err != nil {
		return fmt.Errorf("failed to exec runc: %w\n", err)
	}

	return nil
}

func addHook(spec *specs.Spec) error {
	path, err := exec.LookPath(hookCli)
	if err != nil {
		path = hookDefaultFilePath
		if _, err = os.Stat(path); err != nil {
			return fmt.Errorf("cannot find the hook\n")
		}
	}

	if spec.Hooks == nil {
		spec.Hooks = &specs.Hooks{}
	} else if len(spec.Hooks.Prestart) != 0 {
		for _, hook := range spec.Hooks.Prestart {
			if !strings.Contains(hook.Path, hookCli) {
				continue
			}
			return nil
		}
	}

	spec.Hooks.Prestart = append(spec.Hooks.Prestart, specs.Hook{
		Path: path,
		Args: []string{path},
	})

	return nil
}

func modifySpecFile(path string) error {
	stat, err := os.Stat(path)
	if err != nil {
		return fmt.Errorf("spec file doesnt exist %s: %w", path, err)
	}

	jsonFile, err := os.OpenFile(path, os.O_RDWR, stat.Mode())
	if err != nil {
		return fmt.Errorf("cannot open oci spec file %s: %w\n", path, err)
	}

	defer jsonFile.Close()

	jsonContent, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		return fmt.Errorf("failed to read oci spec file %s: %w\n", path, err)
	}

	var spec specs.Spec
	if err := json.Unmarshal(jsonContent, &spec); err != nil {
		return fmt.Errorf("failed to unmarshal oci spec file %s: %w\n", path, err)
	}

	if err := addHook(&spec); err != nil {
		return fmt.Errorf("failed to inject hook: %w\n", err)
	}

	jsonOutput, err := json.Marshal(spec)
	if err != nil {
		return fmt.Errorf("failed to marshal OCI spec file: %w\n", err)
	}

	if _, err := jsonFile.WriteAt(jsonOutput, 0); err != nil {
		return fmt.Errorf("failed to write OCI spec file: %w\n", err)
	}

	return nil
}

func doProcess() error {
	args, err := getArgs()
	if err != nil {
		return fmt.Errorf("failed to get args: %w\n", err)
	}

	if args.cmd != "create" {
		return execRunc()
	}

	if args.bundleDirPath == "" {
		args.bundleDirPath, err = os.Getwd()
		if err != nil {
			return fmt.Errorf("failed to get current working dir: %w\n", err)
		}
	}

	specFilePath := args.bundleDirPath + "/config.json"

	if err := modifySpecFile(specFilePath); err != nil {
		return fmt.Errorf("failed to modify spec file %s: %w\n", specFilePath, err)
	}

	return execRunc()
}

func main() {
	log.SetPrefix(loggingPrefix)

	if err := doProcess(); err != nil {
		log.Fatal(err)
	}
}

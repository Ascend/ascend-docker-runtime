/* Copyright(C) 2021. Huawei Technologies Co.,Ltd. All rights reserved.
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
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"

	"huawei.com/npu-exporter/v5/common-utils/hwlog"

	"mindxcheckutils"
)

const commonTemplate = `{
        "runtimes":     {
                "ascend":       {
                        "path": "%s",
                        "runtimeArgs":  []
                }
        },
        "default-runtime":      "ascend"
}`

const noDefaultTemplate = `{
        "runtimes":     {
                "ascend":       {
                        "path": "%s",
                        "runtimeArgs":  []
                }
        }
}`

const (
	actionPosition      = 0
	srcFilePosition     = 1
	destFilePosition    = 2
	runtimeFilePosition = 3
	rmCommandLength     = 4
	addCommandLength    = 5
	addCommand          = "add"
	maxCommandLength    = 65535
	logPath             = "/var/log/ascend-docker-runtime/install-helper-run.log"
	rmCommand           = "rm"
	maxFileSize         = 1024 * 1024 * 10
)

var reserveDefaultRuntime = false

func main() {
	ctx, _ := context.WithCancel(context.Background())
	if err := initLogModule(ctx); err != nil {
		log.Fatal(err)
	}
	logPrefixWords, err := mindxcheckutils.GetLogPrefix()
	if err != nil {
		log.Fatal(err)
	}
	hwlog.RunLog.Infof("%v start running script", logPrefixWords)

	if !mindxcheckutils.StringChecker(strings.Join(os.Args, " "), 0,
		maxCommandLength, mindxcheckutils.DefaultWhiteList+" ") {
		hwlog.RunLog.Errorf("%v check command failed, maybe command contains illegal char", logPrefixWords)
		log.Fatalf("command error, please check %s for detail", logPath)
	}

	err, behavior := process()
	if err != nil {
		hwlog.RunLog.Errorf("%v run script failed: %v", logPrefixWords, err)
		log.Fatal(fmt.Errorf("error in installation"))
	}
	hwlog.RunLog.Infof("%v run %v success", logPrefixWords, behavior)
}

func initLogModule(ctx context.Context) error {
	const backups = 2
	const logMaxAge = 365
	logConfig := hwlog.LogConfig{
		LogFileName: logPath,
		LogLevel:    0,
		MaxBackups:  backups,
		MaxAge:      logMaxAge,
		OnlyToFile:  true,
		FileMaxSize: 2,
	}
	if err := hwlog.InitRunLogger(&logConfig, ctx); err != nil {
		fmt.Printf("hwlog init failed, error is %v", err)
		return err
	}
	return nil
}

func checkParamAndGetBehavior(action string, command []string) (bool, string) {
	correctParam, behavior := false, ""
	if action == addCommand && len(command) == addCommandLength {
		correctParam = true
		behavior = "install"
	}
	if action == rmCommand && len(command) == rmCommandLength {
		correctParam = true
		behavior = "uninstall"
	}
	return correctParam, behavior
}

func process() (error, string) {
	const helpMessage = "\tadd <daemon.json path> <daemon.json.result path> <ascend-docker-runtime path>\n" +
		"\t rm <daemon.json path> <daemon.json.result path>\n" +
		"\t -h help command"
	helpFlag := flag.Bool("h", false, helpMessage)
	flag.Parse()
	if *helpFlag {
		_, err := fmt.Println(helpMessage)
		return err, ""
	}
	command := flag.Args()
	if len(command) == 0 {
		return fmt.Errorf("error param"), ""
	}

	action := command[actionPosition]
	correctParam, behavior := checkParamAndGetBehavior(action, command)
	if !correctParam {
		return fmt.Errorf("error param"), ""
	}

	srcFilePath := command[srcFilePosition]
	if _, err := os.Stat(srcFilePath); os.IsNotExist(err) {
		if _, err := mindxcheckutils.RealDirChecker(filepath.Dir(srcFilePath), true, false); err != nil {
			return err, behavior
		}
	} else {
		if _, err := mindxcheckutils.RealFileChecker(srcFilePath, true, false, mindxcheckutils.DefaultSize); err != nil {
			return err, behavior
		}
	}

	destFilePath := command[destFilePosition]
	if _, err := mindxcheckutils.RealDirChecker(filepath.Dir(destFilePath), true, false); err != nil {
		return err, behavior
	}
	runtimeFilePath := ""
	if len(command) == addCommandLength {
		runtimeFilePath = command[runtimeFilePosition]
	}

	setReserveDefaultRuntime(command)

	// check file permission
	writeContent, err := createJsonString(srcFilePath, runtimeFilePath, action)
	if err != nil {
		return err, behavior
	}
	return writeJson(destFilePath, writeContent), behavior
}

func createJsonString(srcFilePath, runtimeFilePath, action string) ([]byte, error) {
	var writeContent []byte
	if _, err := os.Stat(srcFilePath); err == nil {
		daemon, err := modifyDaemon(srcFilePath, runtimeFilePath, action)
		if err != nil {
			return nil, err
		}
		writeContent, err = json.MarshalIndent(daemon, "", "        ")
		if err != nil {
			return nil, err
		}
	} else if os.IsNotExist(err) {
		// not existed
		if !reserveDefaultRuntime {
			writeContent = []byte(fmt.Sprintf(commonTemplate, runtimeFilePath))
		} else {
			writeContent = []byte(fmt.Sprintf(noDefaultTemplate, runtimeFilePath))
		}
	} else {
		return nil, err
	}
	return writeContent, nil
}

func writeJson(destFilePath string, writeContent []byte) error {
	if _, err := os.Stat(destFilePath); os.IsNotExist(err) {
		const perm = 0600
		file, err := os.OpenFile(destFilePath, os.O_CREATE|os.O_RDWR|os.O_TRUNC, perm)
		if err != nil {
			return fmt.Errorf("create target file failed")
		}
		_, err = file.Write(writeContent)
		if err != nil {
			closeErr := file.Close()
			return fmt.Errorf("write target file failed with close err %v", closeErr)
		}
		err = file.Close()
		if err != nil {
			return fmt.Errorf("close target file failed")
		}
		return nil
	} else {
		return fmt.Errorf("target file already existed")
	}
}

func modifyDaemon(srcFilePath, runtimeFilePath, action string) (map[string]interface{}, error) {
	// existed...
	daemon, err := loadOriginJson(srcFilePath)
	if err != nil {
		return nil, err
	}

	if _, ok := daemon["runtimes"]; !ok && action == addCommand {
		daemon["runtimes"] = map[string]interface{}{}
	}
	runtimeValue := daemon["runtimes"]
	runtimeConfig, runtimeConfigOk := runtimeValue.(map[string]interface{})
	if !runtimeConfigOk && action == addCommand {
		return nil, fmt.Errorf("extract runtime failed")
	}
	if action == addCommand {
		if _, ok := runtimeConfig["ascend"]; !ok {
			runtimeConfig["ascend"] = map[string]interface{}{}
		}
		ascendConfig, ok := runtimeConfig["ascend"].(map[string]interface{})
		if !ok {
			return nil, fmt.Errorf("extract ascend failed")
		}
		ascendConfig["path"] = runtimeFilePath
		if _, ok := ascendConfig["runtimeArgs"]; !ok {
			ascendConfig["runtimeArgs"] = []string{}
		}
		if !reserveDefaultRuntime {
			daemon["default-runtime"] = "ascend"
		}
	} else if action == rmCommand {
		if runtimeConfigOk {
			delete(runtimeConfig, "ascend")
		}
		if value, ok := daemon["default-runtime"]; ok && value == "ascend" {
			delete(daemon, "default-runtime")
		}
	} else {
		return nil, fmt.Errorf("param error")
	}
	return daemon, nil
}

func loadOriginJson(srcFilePath string) (map[string]interface{}, error) {
	if fileInfo, err := os.Stat(srcFilePath); err != nil {
		return nil, err
	} else if fileInfo.Size() > maxFileSize {
		return nil, fmt.Errorf("file size too large")
	}

	file, err := os.Open(srcFilePath)
	if err != nil {
		return nil, fmt.Errorf("open daemon.json failed")
	}
	content, err := ioutil.ReadAll(file)
	if err != nil {
		closeErr := file.Close()
		return nil, fmt.Errorf("read daemon.json failed, close file err is %v", closeErr)
	}
	err = file.Close()
	if err != nil {
		return nil, fmt.Errorf("close daemon.json failed")
	}

	var daemon map[string]interface{}
	err = json.Unmarshal(content, &daemon)
	if err != nil {
		return nil, fmt.Errorf("load daemon.json failed")
	}
	return daemon, nil
}

func setReserveDefaultRuntime(command []string) {
	reserveCmdPostion := len(command) - 1
	if command[reserveCmdPostion] == "yes" {
		reserveDefaultRuntime = true
	}
}

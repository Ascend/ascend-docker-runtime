// Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.

// Description: ascend-docker-runtime工具，辅助安装配置
package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"

	"mindxcheckutils"
)

const template = `{
        "runtimes":     {
                "ascend":       {
                        "path": "%s",
                        "runtimeArgs":  []
                }
        },
        "default-runtime":      "ascend"
}`

const (
	actionPosition      = 0
	srcFilePosition     = 1
	destFilePosition    = 2
	runtimeFilePosition = 3
	rmCommandLength     = 3
	addCommandLength    = 4
	addCommand          = "add"
	rmCommand           = "rm"
	maxFileSize         = 1024 * 1024 * 10
)

func main() {
	log.SetPrefix("LOG: ")
	log.Printf("running %s", os.Args)
	err := process()
	if err != nil {
		log.Printf("run %s failed", os.Args)
		log.Fatal(err)
	} else {
		log.Printf("run %s success", os.Args)
	}
}

func process() error {
	const helpMessage = "\tadd <daemon.json path> <daemon.json.result path> <ascend-docker-runtime path>\n" +
		"\t rm <daemon.json path> <daemon.json.result path>\n" +
		"\t -h help command"
	helpFlag := flag.Bool("h", false, helpMessage)
	flag.Parse()
	if *helpFlag {
		_, err := fmt.Println(helpMessage)
		return err
	}
	command := flag.Args()
	if len(command) == 0 {
		return fmt.Errorf("error param")
	}
	action := command[actionPosition]
	correctParam := false
	if action == addCommand && len(command) == addCommandLength {
		correctParam = true
	}
	if action == rmCommand && len(command) == rmCommandLength {
		correctParam = true
	}
	if !correctParam {
		return fmt.Errorf("error param")
	}

	srcFilePath := command[srcFilePosition]
	if _, err := os.Stat(srcFilePath); os.IsNotExist(err) {
		if _, err := mindxcheckutils.FileChecker(filepath.Dir(srcFilePath), true, true, false, 0); err != nil {
			return err
		}
	} else {
		if _, err := mindxcheckutils.FileChecker(srcFilePath, false, true, false, 0); err != nil {
			return err
		}
	}

	destFilePath := command[destFilePosition]
	if _, err := mindxcheckutils.FileChecker(filepath.Dir(destFilePath), true, true, false, 0); err != nil {
		return err
	}
	runtimeFilePath := ""
	if len(command) == addCommandLength {
		runtimeFilePath = command[runtimeFilePosition]
	}

	// check file permission
	writeContent, err := createJsonString(srcFilePath, runtimeFilePath, action)
	if err != nil {
		return err
	}
	return writeJson(destFilePath, writeContent)
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
		writeContent = []byte(fmt.Sprintf(template, runtimeFilePath))
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
		daemon["default-runtime"] = "ascend"
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

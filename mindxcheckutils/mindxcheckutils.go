// Package mindxcheckutils is a check utils package
/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */
package mindxcheckutils

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"syscall"
)

// FileChecker check if a file/dir is safe to use
func FileChecker(path string, allowDir, checkParent, allowLink bool, deep int) (bool, error) {
	const maxDepth, groupWriteIndex, otherWriteIndex, permLength int = 99, 5, 8, 10
	if deep > maxDepth {
		return false, fmt.Errorf("over maxDepth %v", maxDepth)
	}
	if strings.Contains(path, "..") {
		return false, fmt.Errorf("err path %v", path)
	}
	filePath, err := filepath.Abs(path)
	if err != nil {
		return false, fmt.Errorf("get abs path failed %v", err)
	}
	fileInfo, ok, err := normalFileCheck(filePath, allowDir, allowLink)
	if err != nil {
		return ok, err
	}
	perm := fileInfo.Mode().Perm().String()
	if len(perm) != permLength {
		return false, fmt.Errorf("permission not right %v %v", filePath, perm)
	}
	for index, char := range perm {
		switch index {
		case groupWriteIndex, otherWriteIndex:
			if char == 'w' {
				return false, fmt.Errorf("write permission not right %v %v", filePath, perm)
			}
		default:
		}
	}
	stat, ok := fileInfo.Sys().(*syscall.Stat_t)
	if !ok {
		return false, fmt.Errorf("can not get stat %v", filePath)
	}
	uid := int(stat.Uid)
	if !(uid == 0 || uid == os.Getuid()) {
		return false, fmt.Errorf("owner not right %v %v", filePath, uid)
	}
	if filePath != "/" && checkParent {
		return FileChecker(filepath.Dir(filePath), true, true, allowLink, deep+1)
	}
	return true, nil
}

func normalFileCheck(filePath string, allowDir bool, allowLink bool) (os.FileInfo, bool, error) {
	realPath, err := filepath.EvalSymlinks(filePath)
	if err != nil || (realPath != filePath && !allowLink) {
		return nil, false, fmt.Errorf("symlinks or not existed, failed %v, %v", filePath, err)
	}
	fileInfo, err := os.Stat(filePath)
	if err != nil {
		return nil, false, fmt.Errorf("get file stat failed %v", err)
	}
	if allowDir {
		if !fileInfo.Mode().IsRegular() && !fileInfo.IsDir() {
			return nil, false, fmt.Errorf("not regular file/dir %v", filePath)
		}
	} else {
		if !fileInfo.Mode().IsRegular() {
			return nil, false, fmt.Errorf("not regular file %v", filePath)
		}
	}
	if fileInfo.Mode()&os.ModeSetuid != 0 {
		return nil, false, fmt.Errorf("setuid not allowed %v", filePath)
	}
	if fileInfo.Mode()&os.ModeSetgid != 0 {
		return nil, false, fmt.Errorf("setgid not allowed %v", filePath)
	}
	return fileInfo, false, nil
}

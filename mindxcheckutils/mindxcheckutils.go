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

const (
	maxLogFieldLength = 64
	notValidPath      = "not-valid-file-path"
	maxAllowFileSize  = 102400 // in megabytes
	oneMegabytes      = 1048576
	// DefaultSize default size of file allowed
	DefaultSize = 100 // in megabytes
	// DefaultWhiteList default white list in string
	DefaultWhiteList = "-_./~"
	// DefaultStringSize default string max length
	DefaultStringSize = 256
	// DefaultPathSize default string max length
	DefaultPathSize = 4096
)

var logPrefix = ""

// RealFileChecker check if a file is safe to use
func RealFileChecker(path string, checkParent, allowLink bool, size int) (string, error) {
	if !StringChecker(path, 0, DefaultPathSize, DefaultWhiteList) {
		return notValidPath, fmt.Errorf("invalid path")
	}
	_, err := FileChecker(path, false, checkParent, allowLink, 0)
	if err != nil {
		return notValidPath, err
	}
	realPath, err := filepath.Abs(path)
	if err != nil {
		return notValidPath, err
	}
	realPath, err = filepath.EvalSymlinks(realPath)
	if err != nil {
		return notValidPath, err
	}
	fileInfo, err := os.Stat(realPath)
	if err != nil {
		return notValidPath, err
	}
	if !fileInfo.Mode().IsRegular() {
		return notValidPath, fmt.Errorf("invalid regular file")
	}
	if size > maxAllowFileSize || size < 0 {
		return notValidPath, fmt.Errorf("invalid size")
	}
	if fileInfo.Size() > int64(size)*int64(oneMegabytes) {
		return notValidPath, fmt.Errorf("size too large")
	}
	return realPath, nil
}

// RealDirChecker check if a dir is safe to use
func RealDirChecker(path string, checkParent, allowLink bool) (string, error) {
	if !StringChecker(path, 0, DefaultPathSize, DefaultWhiteList) {
		return notValidPath, fmt.Errorf("invalid path")
	}
	_, err := FileChecker(path, true, checkParent, allowLink, 0)
	if err != nil {
		return notValidPath, err
	}
	realPath, err := filepath.Abs(path)
	if err != nil {
		return notValidPath, err
	}
	realPath, err = filepath.EvalSymlinks(realPath)
	if err != nil {
		return notValidPath, err
	}
	fileInfo, err := os.Stat(realPath)
	if err != nil {
		return notValidPath, err
	}
	if !fileInfo.IsDir() {
		return notValidPath, fmt.Errorf("not a dir")
	}
	return realPath, nil
}

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
	if len(filepath.Base(filePath)) > DefaultStringSize {
		return false, fmt.Errorf("path too long")
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

// GetLogPrefix get log prefix
func GetLogPrefix() (string, error) {
	if logPrefix != "" {
		return logPrefix, nil
	}
	uid := os.Geteuid()
	if uid < 0 {
		return "", fmt.Errorf("err: uid not right")
	}
	tty, err := filepath.EvalSymlinks("/proc/self/fd/0")
	if err != nil || !StringChecker(tty, 0, DefaultStringSize, "/:") {
		tty = "unknown"
	}

	logPrefix = fmt.Sprintf("uid: %d tty: %v ", uid, tty)
	return logPrefix, nil
}

func isLetter(c rune) bool {
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')
}

func isNumber(c rune) bool {
	return '0' <= c && c <= '9'
}

func isInWhiteList(c rune, whiteList string) bool {
	return strings.Contains(whiteList, string(c))
}

// StringChecker check string
func StringChecker(text string, minLength, maxLength int, whiteList string) bool {
	if len(text) <= minLength || len(text) >= maxLength {
		return false
	}
	for _, char := range text {
		if isLetter(char) || isNumber(char) || isInWhiteList(char, whiteList) {
			continue
		}
		return false
	}
	return true
}

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

// Package mindxcheckutils
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
	DefaultPathSize               = 4096
	runLogDir                     = "/var/log/ascend-docker-runtime/"
	backupLogFileMode os.FileMode = 0400
	runLogFileMode    os.FileMode = 0750
	maxFileNum                    = 32
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

// ChangeRuntimeLogMode change log mode
func ChangeRuntimeLogMode(runLog, operLog string) error {
	runLogDirLen := len(runLogDir)
	var logMode os.FileMode
	counter := 0
	err := filepath.Walk(runLogDir, func(fileOrPath string, fileInfo os.FileInfo, err error) error {
		counter += 1
		if counter > maxFileNum {
			return fmt.Errorf("the counter file is over maxFileNum")
		}
		if err != nil {
			fmt.Printf("prevent panic by handling failure accessing a path %q: %v\n", fileOrPath, err)
			return err
		}
		hasLogPrefix := strings.HasPrefix(fileOrPath[runLogDirLen:],
			runLog) || strings.HasPrefix(fileOrPath[runLogDirLen:], operLog)
		if !hasLogPrefix {
			return nil
		}
		logMode = backupLogFileMode
		if fileInfo.Mode()&os.ModeSymlink == os.ModeSymlink {
			return fmt.Errorf("the file or path is symlink")
		}
		if errChmod := os.Chmod(fileOrPath, logMode); errChmod != nil {
			return fmt.Errorf("set file mode %s failed", fileOrPath)
		}
		return nil
	})
	if err != nil {
		return fmt.Errorf("traversal runLogDir failed")
	}
	return nil
}

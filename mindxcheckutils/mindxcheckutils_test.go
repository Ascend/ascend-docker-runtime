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
	"os"
	"strings"
	"testing"
)

func TestNormalFileCheckRegularFile(t *testing.T) {
	tmpDir, filePath, err := createTestFile(t, "test_file.txt")
	defer removeTmpDir(t, tmpDir)
	err = os.Symlink(filePath, tmpDir+"/syslink")
	if err != nil {
		t.Fatalf("create symlink failed %q: %s", filePath, err)
	}

	if _, _, err = normalFileCheck(tmpDir, true, false); err != nil {
		t.Fatalf("check allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck(tmpDir, false, false); !strings.Contains(err.Error(), "not regular file") {
		t.Fatalf("check not allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck("/dev/zero", true, false); !strings.Contains(err.Error(), "not regular file/dir") {
		t.Fatalf("check /dev/zero failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck(tmpDir+"/syslink", false, false); !strings.Contains(err.Error(), "symlinks") {
		t.Fatalf("check symlinks failed %q: %s", tmpDir+"/syslink", err)
	}

	if _, _, err = normalFileCheck(filePath, false, false); err != nil {
		t.Fatalf("check failed %q: %s", filePath, err)
	}

	if _, _, err = normalFileCheck(tmpDir+"/notexisted", false, false); !strings.Contains(err.Error(), "not existed") {
		t.Fatalf("check symlinks failed %q: %s", tmpDir+"/syslink", err)
	}
}

func TestFileCheckRegularFile(t *testing.T) {
	tmpDir, filePath, err := createTestFile(t, "test_file.txt")
	defer removeTmpDir(t, tmpDir)
	err = os.Symlink(filePath, tmpDir+"/syslink")
	if err != nil {
		t.Fatalf("create symlink failed %q: %s", filePath, err)
	}

	if _, err = FileChecker(tmpDir, true, false, false, 0); err != nil {
		t.Fatalf("check allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, err = FileChecker(tmpDir, false, false, false, 0); err != nil &&
		!strings.Contains(err.Error(), "not regular file") {
		t.Fatalf("check not allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, err = FileChecker("/dev/zero", true, false, false, 0); err != nil &&
		!strings.Contains(err.Error(), "not regular file/dir") {
		t.Fatalf("check /dev/zero failed %q: %s", tmpDir+"/__test__", err)
	}
}

func TestGetLogPrefix(t *testing.T) {
	logPrefix = ""
	prefix, err := GetLogPrefix()
	if err != nil {
		t.Fatalf("get log prefix failed %v %v", prefix, err)
	}
	if logPrefix == "" || prefix != logPrefix {
		t.Fatalf("get log prefix failed 2 %v %v", prefix, prefix)
	}
}

func TestRealFileChecker(t *testing.T) {
	tmpDir, filePath, err := createTestFile(t, "test_file.txt")
	if err != nil {
		t.Fatalf("create file failed %q: %s", filePath, err)
	}
	defer removeTmpDir(t, tmpDir)
	const permission os.FileMode = 0700
	err = os.WriteFile(filePath, []byte("hello\n"), permission)
	if err != nil {
		t.Fatalf("create file failed %q: %s", filePath, err)
	}
	if _, err = RealFileChecker(filePath, false, true, 0); err == nil {
		t.Fatalf("size check wrong 0 %q: %s", filePath, err)
	}
	if _, err = RealFileChecker(filePath, false, true, 1); err != nil {
		t.Fatalf("size check wrong 1 %q: %s", filePath, err)
	}
}

func TestRealDirChecker(t *testing.T) {
	tmpDir, filePath, err := createTestFile(t, "test_file.txt")
	if err != nil {
		t.Fatalf("create file failed %q: %s", filePath, err)
	}
	defer removeTmpDir(t, tmpDir)
	if _, err = RealDirChecker(filePath, false, true); err == nil {
		t.Fatalf("should be dir 0 %q: %s", filePath, err)
	}
	if _, err = RealDirChecker(tmpDir, false, true); err != nil {
		t.Fatalf("should be dir 1 %q: %s", filePath, err)
	}
}

func TestStringChecker(t *testing.T) {
	if ok := StringChecker("0123456789abcABC", 0, DefaultStringSize, ""); !ok {
		t.Fatalf("failed on regular letters")
	}
	const testSize = 3
	if ok := StringChecker("123", 0, testSize, ""); ok {
		t.Fatalf("failed on max length")
	}
	if ok := StringChecker("1234", 0, testSize, ""); ok {
		t.Fatalf("failed on max length")
	}
	if ok := StringChecker("12", 0, testSize, ""); !ok {
		t.Fatalf("failed on max length")
	}
	if ok := StringChecker("", 0, testSize, ""); ok {
		t.Fatalf("failed on min length")
	}
	if ok := StringChecker("123", testSize, DefaultStringSize, ""); ok {
		t.Fatalf("failed on min length")
	}
	if ok := StringChecker("123%", 0, DefaultStringSize, ""); ok {
		t.Fatalf("failed on strange words")
	}
	if ok := StringChecker("123.-/~", 0, DefaultStringSize, DefaultWhiteList); !ok {
		t.Fatalf("failed on strange words")
	}
}

func createTestFile(t *testing.T, fileName string) (string, string, error) {
	tmpDir := os.TempDir()
	const permission os.FileMode = 0700
	if os.MkdirAll(tmpDir+"/__test__", permission) != nil {
		t.Fatalf("MkdirAll failed %q", tmpDir+"/__test__")
	}
	_, err := os.Create(tmpDir + "/__test__" + fileName)
	if err != nil {
		t.Fatalf("create file failed %q: %s", tmpDir+"/__test__", err)
	}
	return tmpDir + "/__test__", tmpDir + "/__test__" + fileName, err
}

func removeTmpDir(t *testing.T, tmpDir string) {
	if os.RemoveAll(tmpDir) != nil {
		t.Logf("removeall %v", tmpDir)
	}
}

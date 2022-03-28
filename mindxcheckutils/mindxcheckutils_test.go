/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

// Package mindxcheckutils is a check utils package
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

	if _, _, err = normalFileCheck(tmpDir, true); err != nil {
		t.Fatalf("check allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck(tmpDir, false); !strings.Contains(err.Error(), "not regular file") {
		t.Fatalf("check not allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck("/dev/zero", true); !strings.Contains(err.Error(), "not regular file/dir") {
		t.Fatalf("check /dev/zero failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, _, err = normalFileCheck(tmpDir+"/syslink", false); !strings.Contains(err.Error(), "symlinks") {
		t.Fatalf("check symlinks failed %q: %s", tmpDir+"/syslink", err)
	}

	if _, _, err = normalFileCheck(filePath, false); err != nil {
		t.Fatalf("check failed %q: %s", filePath, err)
	}

	if _, _, err = normalFileCheck(tmpDir+"/notexisted", false); !strings.Contains(err.Error(), "not existed") {
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

	if _, err = FileChecker(tmpDir, true, false, 0); err != nil {
		t.Fatalf("check allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, err = FileChecker(tmpDir, false, false, 0); err != nil && !strings.Contains(err.Error(), "not regular file") {
		t.Fatalf("check not allow dir failed %q: %s", tmpDir+"/__test__", err)
	}

	if _, err = FileChecker("/dev/zero", true, false, 0); err != nil &&
		!strings.Contains(err.Error(), "not regular file/dir") {
		t.Fatalf("check /dev/zero failed %q: %s", tmpDir+"/__test__", err)
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

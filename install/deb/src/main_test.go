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
	"encoding/json"
	"os"
	"reflect"
	"testing"
)

const oldString = `{
        "runtimes":     {
                "ascend":       {
                        "path": "/test/runtime",
                        "runtimeArgs":  []
                }
        },
        "default-runtime":      "ascend"
	}`

func jSONBytesEqual(a, b []byte) (bool, error) {
	var contentA, contentB interface{}
	if err := json.Unmarshal(a, &contentA); err != nil {
		return false, err
	}
	if err := json.Unmarshal(b, &contentB); err != nil {
		return false, err
	}
	return reflect.DeepEqual(contentB, contentA), nil
}

func TestCreateJsonStringWholeNew(t *testing.T) {
	data, err := createJsonString("/notExistedFile", "/test/runtime", "add")
	if err != nil {
		t.Fatalf("create string failed %s", err)
	}

	if eq, err := jSONBytesEqual([]byte(oldString), data); err != nil || !eq {
		t.Fatalf("empty create equal failed %s, %v", err, string(data))
	}
}

func TestCreateJsonStringUpdate(t *testing.T) {
	const perm = 0600
	if fid, err := os.OpenFile("old.json", os.O_CREATE|os.O_RDWR|os.O_TRUNC, perm); err == nil {
		_, err = fid.Write([]byte(oldString))
		closeErr := fid.Close()
		if err != nil || closeErr != nil {
			t.Fatalf("create old failed %s", err)
		}
	}
	data, err := createJsonString("old.json", "/test/runtime1", "add")
	if err != nil {
		t.Fatalf("update failed %s", err)
	}
	expectString := `{
        "runtimes":     {
                "ascend":       {
                        "path": "/test/runtime1",
                        "runtimeArgs":  []
                }
        },
        "default-runtime":      "ascend"
	}`
	if eq, err := jSONBytesEqual([]byte(expectString), data); err != nil || !eq {
		t.Fatalf("update failed %s, %v", err, string(data))
	}
}

func TestCreateJsonStringUpdateWithOtherParam(t *testing.T) {
	const perm = 0600
	oldStringWithParam := `{
        "runtimes":     {
                "ascend":       {
                        "path": "/test/runtime",
                        "runtimeArgs":  [1,2,3]
                },
				"runc2":       {
                        "path": "/test/runtime2",
                        "runtimeArgs":  [1,2,3]
                }
        },
        "default-runtime":      "runc"
	}`
	if fid, err := os.OpenFile("old.json", os.O_CREATE|os.O_RDWR|os.O_TRUNC, perm); err == nil {
		_, err = fid.Write([]byte(oldStringWithParam))
		closeErr := fid.Close()
		if err != nil || closeErr != nil {
			t.Fatalf("create old failed %s", err)
		}
	}
	data, err := createJsonString("old.json", "/test/runtime1", "add")
	if err != nil {
		t.Fatalf("update failed %s", err)
	}
	expectString := `{
        "runtimes":     {
                "ascend":       {
                        "path": "/test/runtime1",
                        "runtimeArgs":  [1,2,3]
                },
				"runc2":       {
                        "path": "/test/runtime2",
                        "runtimeArgs":  [1,2,3]
                }
        },
        "default-runtime":      "ascend"
	}`
	if eq, err := jSONBytesEqual([]byte(expectString), data); err != nil || !eq {
		t.Fatalf("update failed %s, %v", err, string(data))
	}
}

func TestCreateJsonStrinRm(t *testing.T) {
	const perm = 0600
	if fid, err := os.OpenFile("old.json", os.O_CREATE|os.O_RDWR|os.O_TRUNC, perm); err == nil {
		_, err = fid.Write([]byte(oldString))
		closeErr := fid.Close()
		if err != nil || closeErr != nil {
			t.Fatalf("create old failed %s", err)
		}
	}
	data, err := createJsonString("old.json", "", "rm")
	if err != nil {
		t.Fatalf("update failed %s", err)
	}
	expectString := `{
        "runtimes":     {}
	}`
	if eq, err := jSONBytesEqual([]byte(expectString), data); err != nil || !eq {
		t.Fatalf("update failed %s, %v", err, string(data))
	}
}

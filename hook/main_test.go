/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hook main 函数单元测试
*/
package main

import (
	"reflect"
	"testing"
)

func TestRemoveDuplication(t *testing.T) {
	originList := []int {1,2,2,4,5,5,5,6,8,8}
	targetList := []int {1,2,4,5,6,8}
	resultList := removeDuplication(originList)

	if !reflect.DeepEqual(resultList, targetList) {
		t.Fail()
	}
}

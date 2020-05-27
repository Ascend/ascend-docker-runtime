package main

import (
	"reflect"
	"testing"
)

func TestRemoveDuplication(t *testing.T) {
	originList := []int {1,2,2,4,5,5,5,6,8,8}
	resultList := removeDuplication(originList)

	if !reflect.DeepEqual(resultList, []int {1,2,4,5,6,8}) {
		t.Fail()
	}
}

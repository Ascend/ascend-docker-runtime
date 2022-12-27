module main

go 1.17

require (
	huawei.com/npu-exporter/v3 v3.0.0
	mindxcheckutils v1.0.0
)

require (
	github.com/fsnotify/fsnotify v1.6.0 // indirect
	github.com/gopherjs/gopherjs v1.17.2 // indirect
	github.com/jtolds/gls v4.20.0+incompatible // indirect
	github.com/smartystreets/assertions v1.13.0 // indirect
	golang.org/x/sys v0.0.0-20220908164124-27713097b956 // indirect
)

replace (
	huawei.com/npu-exporter/v3 => gitee.com/ascend/ascend-npu-exporter/v3 v3.0.0
	mindxcheckutils => ../../../mindxcheckutils
)

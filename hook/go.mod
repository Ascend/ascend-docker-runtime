module main

go 1.17

require (
	github.com/opencontainers/runtime-spec v1.0.3-0.20220718201635-a8106e99982b
	github.com/prashantv/gostub v1.1.0
	huawei.com/npu-exporter/v5 v5.0.0-rc1.1
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
	huawei.com/npu-exporter/v5 => gitee.com/ascend/ascend-npu-exporter/v5 v5.0.0-rc1.1
	mindxcheckutils => ../mindxcheckutils
)

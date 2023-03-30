module main

go 1.17

require (
	github.com/containerd/containerd v1.6.19
	github.com/opencontainers/runtime-spec v1.0.3-0.20220718201635-a8106e99982b
	github.com/prashantv/gostub v1.1.0
	huawei.com/npu-exporter/v5 v5.0.0-rc1.1
	mindxcheckutils v1.0.0
)

require (
	github.com/Microsoft/go-winio v0.5.2 // indirect
	github.com/Microsoft/hcsshim v0.9.7 // indirect
	github.com/containerd/cgroups v1.0.4 // indirect
	github.com/containerd/continuity v0.3.0 // indirect
	github.com/containerd/ttrpc v1.1.0 // indirect
	github.com/fsnotify/fsnotify v1.6.0 // indirect
	github.com/gogo/protobuf v1.3.2 // indirect
	github.com/golang/groupcache v0.0.0-20210331224755-41bb18bfe9da // indirect
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/gopherjs/gopherjs v1.17.2 // indirect
	github.com/klauspost/compress v1.11.13 // indirect
	github.com/moby/sys/mountinfo v0.5.0 // indirect
	github.com/opencontainers/go-digest v1.0.0 // indirect
	github.com/opencontainers/image-spec v1.0.3-0.20211202183452-c5a74bcca799 // indirect
	github.com/opencontainers/runc v1.1.2 // indirect
	github.com/pkg/errors v0.9.1 // indirect
	github.com/sirupsen/logrus v1.8.1 // indirect
	github.com/smartystreets/assertions v1.13.0 // indirect
	github.com/stretchr/testify v1.8.0 // indirect
	go.opencensus.io v0.23.0 // indirect
	golang.org/x/sync v0.0.0-20210220032951-036812b2e83c // indirect
	golang.org/x/sys v0.0.0-20220908164124-27713097b956 // indirect
	google.golang.org/genproto v0.0.0-20220502173005-c8bf987b8c21 // indirect
	google.golang.org/grpc v1.47.0 // indirect
	google.golang.org/protobuf v1.28.0 // indirect
)

replace (
	github.com/prashantv/gostub => github.com/prashantv/gostub v1.0.1-0.20191007164320-bbe3712b9c4a
	huawei.com/npu-exporter/v5 => gitee.com/ascend/ascend-npu-exporter/v5 v5.0.0-rc1.1
	mindxcheckutils => ../mindxcheckutils
)

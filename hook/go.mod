module main

go 1.16

require (
	github.com/opencontainers/runtime-spec v1.0.2
	github.com/prashantv/gostub v1.1.0
	huawei.com/npu-exporter v0.2.7
	mindxcheckutils v1.0.0
)

replace (
	mindxcheckutils => ../mindxcheckutils
	huawei.com/kmc => codehub-dg-y.huawei.com/it-edge-native/edge-native-core/coastguard.git v1.0.6
	huawei.com/npu-exporter => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/npu-exporter.git v0.2.7
)

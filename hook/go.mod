module main

go 1.16

require (
	github.com/opencontainers/runtime-spec v1.0.3-0.20220718201635-a8106e99982b
	github.com/prashantv/gostub v1.1.0
	huawei.com/mindx/common/hwlog v0.0.0
	mindxcheckutils v1.0.0
)

replace (
	huawei.com/mindx/common/cache => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/cache v0.0.2
	huawei.com/mindx/common/hwlog => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/hwlog v0.0.10
	huawei.com/mindx/common/utils => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/utils v0.0.6
	mindxcheckutils => ../mindxcheckutils
)

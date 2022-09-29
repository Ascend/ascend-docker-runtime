module main

go 1.16

require (
	github.com/opencontainers/runtime-spec v1.0.3-0.20220718201635-a8106e99982b
	github.com/prashantv/gostub v1.1.0
	github.com/stretchr/testify v1.8.0 // indirect
	huawei.com/mindx/common/hwlog v0.0.0
	mindxcheckutils v1.0.0
)

replace (
	github.com/prashantv/gostub => github.com/prashantv/gostub v1.0.1-0.20191007164320-bbe3712b9c4a
	huawei.com/mindx/common/hwlog => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/hwlog v0.0.3
	huawei.com/mindx/common/utils => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/utils v0.0.6
	mindxcheckutils => ../mindxcheckutils
)

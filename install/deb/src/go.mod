module main

go 1.16

require (
	huawei.com/mindx/common/hwlog v0.0.0
	mindxcheckutils v1.0.0
)

replace (
	huawei.com/mindx/common/hwlog => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/hwlog v0.0.3
	huawei.com/mindx/common/utils => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/utils v0.0.6
	mindxcheckutils => ../../../mindxcheckutils
)

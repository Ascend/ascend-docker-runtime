module main

go 1.16

require (
    huawei.com/mindx/common/hwlog v0.0.0
	mindxcheckutils v1.0.0
)

replace (
	mindxcheckutils => ../../../mindxcheckutils
	huawei.com/kmc => codehub-dg-y.huawei.com/it-edge-native/edge-native-core/coastguard.git v1.0.6
	huawei.com/mindx/common/hwlog => codehub-dg-y.huawei.com/MindX_DL/AtlasEnableWarehouse/common-utils.git/hwlog v0.0.3
	k8s.io/api v0.0.0 => codehub-dg-y.huawei.com/OpenSourceCenter/kubernetes.git/staging/src/k8s.io/api v1.19.4-h4
	k8s.io/apimachinery => codehub-dg-y.huawei.com/OpenSourceCenter/kubernetes.git/staging/src/k8s.io/apimachinery v1.19.4-h4
	k8s.io/client-go => codehub-dg-y.huawei.com/OpenSourceCenter/kubernetes.git/staging/src/k8s.io/client-go v1.19.4-h4
	k8s.io/cri-api => codehub-dg-y.huawei.com/OpenSourceCenter/kubernetes.git/staging/src/k8s.io/cri-api v1.19.4-h4
)
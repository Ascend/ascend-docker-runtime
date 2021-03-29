
 # 简介
容器引擎插件（Ascend Docker，又叫昇腾容器）是CANN的基础组件，为所有的AI训练/推理作业提供Ascend NPU（昇腾处理器）容器化支持，使用户AI作业能够以Docker容器的方式平滑运行在昇腾设备之上，如图2-1所示。Ascend Docker配套发布的软件包为Ascend-docker-runtime，已集成至实用工具包toolbox中。
图2-1 Ascend Docker
![image](/uploads/cd30f5e0-d6bd-4838-8d3d-c2a1aa5fcd0b/1616986155878.png '1616986155878.png')
## Ascend Docker价值
- 充分解耦：与Docker解耦，无需修改Docker代码，Runtime可以独立演进。
- 后向兼容：提供可选装的Runtime，不影响原生Docker使用方式。
- 易适配：与客户现有平台和系统平滑适配，不影响原Docker的命令接口。
- 易部署：提供rpm包部署，用户安装后即可用Docker创建挂载Ascend NPU的容器。

## Ascend Docker设计简介
Ascend Docker本质上是基于OCI标准实现的Docker Runtime，不修改Docker引擎，对Docker以插件方式提供Ascend NPU适配功能。
如图2-2所示，Ascend Docker通过OCI接口与原生Docker对接。在原生Docker的runc启动容器过程中，会调用prestart-hook对容器进行配置管理。
图2-2 Docker适配原理
![image](/uploads/5a5617b5-a9ef-4df2-b023-bd641d77257f/1616986181176.png '1616986181176.png')
其中，prestart-hook是OCI定义的容器生存状态，即created状态到running状态的一个中间过渡所设置的钩子函数。在这个过渡状态，容器的namespace已经被创建，但容器的作业还没有启动，因此可以对容器进行设备挂载，cgroup配置等操作。这样随后启动的作业便可以使用到这些配置。
Ascend Docker在prestart-hook这个钩子函数中，对容器做了以下配置操作：
1.根据ASCEND_VISIBLE_DEVICES，将对应的NPU设备挂载到容器的namespace。
2.在Host上配置该容器的device cgroup，确保该容器只可以使用指定的NPU，保证设备的隔离。
3.将Host上的CANN Runtime Library挂载到容器的namespace。

# 发行说明
## Ascend-Docker-Runtime2.0
### 更新说明
 - 容器内设备序号保持不变
 - 添加NODRV选项不挂载驱动目录
 - 适配驱动--run安装模式
 - 适配训练驱动npu-smi位置从sbin移动到bin
 - 挂载列表可配置
 - 适配海思驱动移除add-ons目录修改
 - 跳过不存在的挂载项
 
### 约束
 - Ascend-Docker-Runtime暂不支持Atlas 200 AI加速模块（RC场景）和Atlas 500智能小站

# 下载和安装
## 安装前准备

- 安装大于18.03版本的docker

- 安装驱动:下载链接为https://ascend.huawei.com/zh/#/software/cann/commercial

- 安装固件:下载链接为https://ascend.huawei.com/zh/#/software/cann/commercial



## 下载
### run包
开发人员可从昇腾社区下载Toolbox,下载链接为：https://ascend.huawei.com/zh/#/software/cann/commercial，
下载后安装Toolbox，Ascend-docker-runtime，已集成至实用工具包toolbox中。

# 功能
## 默认挂载目录
| 挂载项       | 备注        |
|:-----------:| :-------------:|
|/dev/davinciX|NPU设备，X是物理ID号例如davinci0|
|/dev/davinci_manager |管理设备|
| /dev/devmm_svm| 管理设备|
|/dev/hisi_hdc | 管理设备|
|/usr/local/Ascend/driver/{lib64, include, tools}  |驱动目录|
 |/usr/local/dcmi | DCMI目录|
|/usr/local/bin/npu-smi | npu-smi工具|
## Ascend-Docker-runtime安装
单独安装
```
chmod +x Ascend-cann-toolbox*.run
./Ascend-cann-toolbox*.run  --install --whitelist=docker-runtime
systemctl daemon-reload
systenctl restart docker
```
集成工具toolboox安装
```
chmod +x Ascend-cann-toolbox*.run
./Ascend-cann-toolbox*.run  --install
systemctl daemon-reload
systemctl restart docker
```
## 挂载单芯片
例子：
```
docker run -it -e ASCEND_VISIBLE_DEVICES=0 imageId /bin/bash
```
imageId 替换为实际镜像名或者ID

检查挂载成功：
```
ls /dev | grep davinci* && ls /dev | grep devmm_svm && ls /dev | grep hisi_hdc && ls /usr/local/Ascend/driver && ls /usr/local/ |grep dcmi && ls /usr/local/bin
```
![image](/uploads/1e77fb8a-f2ca-4768-a331-934b3da2de44/1616986864390.png '1616986864390.png')

ASCEND_VISIBLE_DEVICES=0，参数0替换为要挂载的芯片物理ID
## 挂载多芯片
例子：
```
docker run --rm -it -e ASCEND_VISIBLE_DEVICES=0-3 imageId /bin/bash
或者
docker run --rm -it -e ASCEND_VISIBLE_DEVICES=0,1,2,3 imageId /bin/bash
或者
docker run --rm -it -e ASCEND_VISIBLE_DEVICES=0-2,3 imageId /bin/bash
```
检查挂载成功：
```
ls /dev | grep davinci* && ls /dev | grep devmm_svm && ls /dev | grep hisi_hdc && ls /usr/local/Ascend/driver && ls /usr/local/ |grep dcmi && ls /usr/local/bin
```
![image](/uploads/96fa6aff-59f9-48bb-ba52-13dcf0ff649b/1616986959639.png '1616986959639.png')
## 在默认的挂载的基础上新增挂载内容
```
在/etc/ascend-docker-runtime.d目录下新建挂载文件xxx.list，内容格式例子：
/usr/bin/curl
/usr/bin/gcc
...

在命令中使用文件例子：
docker run --rm -it -e ASCEND_VISIBLE_DEVICES=0 -e ASCEND_RUNTIME_MOUNTS=xxx imageId /bin/bash
```
xxx是新增挂载内容文件名，文件名必须是小写

检查挂载成功：

![image](/uploads/a8f82fea-4075-45e3-b5a1-d9aa7dec267a/1616987709657.png '1616987709657.png')
## 卸载

集成工具toolboox安装
```
./Ascend-cann-toolbox*.run --uninstall
```
## 升级

集成工具toolboox安装方式升级
```
./Ascend-cann-toolbox*.run --upgrade
```


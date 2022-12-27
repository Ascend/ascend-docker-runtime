# 使用测试用例

进入Ascend Docker Runtime的根目录
```shell
cd <path-to-root-path-of-ascend-docker-runtime>
```
下载相关的开源库，注意对应的tag
```shell
git clone -b v2.7 https://github.com/sinojelly/mockcpp.git
git clone -b release-1.10.0 https://github.com/google/googletest.git
git clone -b v1.1.10 https://gitee.com/openeuler/libboundscheck.git
```
进入dt目录
```shell
cd cli/test/dt
```
编译
```shell
sh build.sh
```
结果会显示相关的测试用例覆盖率
```shell
...
Writing directory view page.
Overall coverage rate:
  lines......: 63.3% (746 of 1179 lines)
  functions..: 91.4% (74 of 81 functions)
  branches...: 54.0% (409 of 758 branches)
-------------run_ut cli end-------------------
run_lcov_cli succeed.
```
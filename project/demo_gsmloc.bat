@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=gsmloc
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U
REM BAT_BUILD_APP_HTTP只有等于ON才会启动编译，区分大小写。其他值或者不设置，均为不参与编译
set BAT_BUILD_APP_HTTP=ON
call ..\tools\core_launch.bat gsmloc

REM/*+\NEW\chenzhimin\2020.07.22\*/
cd %PROJECT_OUT%
cmake ..\.. -G Ninja ^
-D CONFIG_BUILD_APP_HTTP=ON
ninja
cd ..\..\project
REM/*-\NEW\chenzhimin\2020.07.22\*/
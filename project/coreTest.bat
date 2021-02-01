@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=coreTest
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U_FS
call ..\tools\core_launch.bat coreTest

cd %PROJECT_OUT%
cmake ..\.. -G Ninja ^
-D CONFIG_BUILD_APP_FTP=ON ^
-D CONFIG_BUILD_APP_HTTP=ON
ninja
cd ..\..\project
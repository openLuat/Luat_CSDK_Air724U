@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=ftp
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U
REM BAT_BUILD_APP_FTP只有等于ON才会启动编译，区分大小写。其他值或者不设置，均为不参与编译
set BAT_BUILD_APP_FTP=ON
call ..\tools\core_launch.bat ftp

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

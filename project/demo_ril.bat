@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=ril
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U
call ..\tools\core_launch.bat ril

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

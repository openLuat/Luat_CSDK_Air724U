@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=2_POC_KEY
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U
call ..\tools\core_launch.bat %CSDK_PRO%

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

@echo off
set CSDK_VER=1.0.0
set FOTA_FLAG=%1
set FOTA_TYPE=%2

call ..\tools\core_launch.bat i2c

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project
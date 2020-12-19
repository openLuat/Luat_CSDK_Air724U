@echo off
set CSDK_VER=1.0.0
set CSDK_PRO=tts
set FOTA_FLAG=%1
set FOTA_TYPE=%2
set AM_MODEL=iot_SDK_720U_BT_TTS

call ..\tools\core_launch.bat tts 

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

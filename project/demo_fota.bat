@echo off

call ..\tools\core_launch.bat fota

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

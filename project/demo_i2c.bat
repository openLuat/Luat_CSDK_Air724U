@echo off

call ..\tools\core_launch.bat i2c

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

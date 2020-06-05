@echo off

call ..\tools\core_launch.bat uart

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

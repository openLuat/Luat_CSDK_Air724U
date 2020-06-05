@echo off

call ..\tools\core_launch.bat flash

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

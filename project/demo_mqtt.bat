@echo off

call ..\tools\core_launch.bat mqtt

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

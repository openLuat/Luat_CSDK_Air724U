@echo off

call ..\tools\core_launch.bat zbar

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

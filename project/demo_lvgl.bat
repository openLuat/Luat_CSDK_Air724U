@echo off

call ..\tools\core_launch.bat ui

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project
@echo off

call ..\tools\core_launch.bat socket

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

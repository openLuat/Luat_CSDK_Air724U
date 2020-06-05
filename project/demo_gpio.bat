@echo off

call ..\tools\core_launch.bat gpio

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

@echo off

call ..\tools\core_launch.bat datetime

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

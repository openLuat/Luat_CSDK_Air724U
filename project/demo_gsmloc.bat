@echo off

call ..\tools\core_launch.bat gsmloc

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

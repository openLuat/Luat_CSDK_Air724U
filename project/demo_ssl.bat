@echo off

call ..\tools\core_launch.bat ssl

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

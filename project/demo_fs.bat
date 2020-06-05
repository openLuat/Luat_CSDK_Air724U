@echo off

call ..\tools\core_launch.bat fs

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

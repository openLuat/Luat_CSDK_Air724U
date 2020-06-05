@echo off

call ..\tools\core_launch.bat ftp

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

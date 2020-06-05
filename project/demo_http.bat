@echo off

call ..\tools\core_launch.bat http

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

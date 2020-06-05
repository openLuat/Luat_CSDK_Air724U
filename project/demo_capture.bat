@echo off

call ..\tools\core_launch.bat capture

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

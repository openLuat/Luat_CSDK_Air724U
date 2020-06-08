@echo off

call ..\tools\core_launch.bat audio

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

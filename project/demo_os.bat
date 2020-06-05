@echo off

call ..\tools\core_launch.bat os

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

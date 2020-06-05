@echo off

call ..\tools\core_launch.bat hello

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

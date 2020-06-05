@echo off

call ..\tools\core_launch.bat vat

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

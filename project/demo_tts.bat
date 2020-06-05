@echo off

call ..\tools\core_launch.bat tts

cd %PROJECT_OUT% & cmake ..\.. -G Ninja & ninja & cd ..\..\project

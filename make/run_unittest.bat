@echo OFF

pushd %~dp0

start /B /WAIT /D ..\data ..\build\Release\emulators\emulators.exe -unittest

popd


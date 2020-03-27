@echo OFF

pushd %~dp0

if not exist ..\test_results (
	mkdir ..\test_results
)

start /B /WAIT /D ..\data ..\build\Release\emulators\emulators.exe -unittest

popd


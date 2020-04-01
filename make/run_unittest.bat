@echo OFF

pushd %~dp0

rmdir /s /q ..\test_results
mkdir ..\test_results
	
start /B /WAIT /D ..\data ..\build\Release\emulators\emulators.exe -unittest

cd ..\data

for %%i in (*.txt) do (
	mkdir ..\test_results\%%~ni
	copy %%i ..\test_results\%%~ni\
)

popd
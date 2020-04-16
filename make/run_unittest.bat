@echo OFF

pushd %~dp0

rmdir /s /q ..\_test_results
mkdir ..\_test_results
	
start /B /WAIT /D ..\data ..\_build\Release\emulators\emulators.exe

popd
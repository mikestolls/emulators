@echo OFF

pushd %~dp0

"..\buil\Release\emulators\emulators.exe -unittest ..\..\..\data\gameboy\unit_test.json"

popd


@echo OFF

pushd %~dp0

"..\build\Release\emulators\emulators.exe -unittest ..\data\gameboy\unit_test.json"

popd


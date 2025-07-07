@echo OFF

pushd %~dp0

cmake -S ..\ -B ..\_prj

popd

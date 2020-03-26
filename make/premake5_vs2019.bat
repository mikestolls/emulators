@echo OFF

pushd %~dp0

call premake5 vs2019

popd

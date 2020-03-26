@echo OFF

pushd %~dp0

call premake5 vs2015

popd
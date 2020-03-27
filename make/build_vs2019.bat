@echo OFF

pushd %~dp0

msbuild "..\prj\vs2019\emulators.sln" -p:Configuration=Release

popd


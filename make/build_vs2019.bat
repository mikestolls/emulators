@echo OFF

pushd %~dp0

call msbuild "..\prj\vs2019\emulators.sln" -p:Configuration=Release

popd


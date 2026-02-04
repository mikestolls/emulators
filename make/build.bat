@echo OFF

pushd %~dp0

msbuild "..\_prj\emulators.sln" -p:Configuration=Release

popd


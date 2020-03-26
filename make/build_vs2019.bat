@echo OFF

call msbuild ".\prj\vs2019\emulators.sln" -p:Configuration=Release

pause
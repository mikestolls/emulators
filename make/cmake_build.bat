@echo OFF

pushd %~dp0

REM Clean old build if it exists
if exist "..\_prj" rmdir /s /q "..\_prj"

REM Configure CMake with Visual Studio 2022
cmake -S ..\ -B ..\_prj -G "Visual Studio 18 2026" -A x64

REM Build the project
cmake --build ..\_prj --config Debug

echo.
echo Build complete! Executable located in: ..\_build\Debug\emulators.exe
echo.

popd
pause
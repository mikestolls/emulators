@echo OFF

pushd %~dp0

..\tools\7zip\7z.exe a ..\_artifacts\emulators_vs2019.zip ..\_build\Release\emulators\*
..\tools\7zip\7z.exe a ..\_artifacts\emulators_vs2019.zip ..\data\*

popd


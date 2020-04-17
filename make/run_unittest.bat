@echo OFF

pushd %~dp0

rmdir /s /q ..\_test_results
mkdir ..\_test_results

python3 run_unittest.py --emulator ..\_build\Release\emulators\emulators.exe --unit_test_filename ..\data\gameboy\unit_test.json --results_dir ..\_test_results

popd
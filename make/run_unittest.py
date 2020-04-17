import json 
import datetime
import sys
import os
import shutil
import argparse
import subprocess
import time

from junit_xml import TestSuite, TestCase

if __name__ == "__main__":
	
	parser = argparse.ArgumentParser(description='Run a unit test')
	parser.add_argument('--emulator', required=True, help='The emulator executable to run the unit test')
	parser.add_argument('--unit_test_filename', required=True, help='Unit test filename')
	parser.add_argument('--results_dir', required=True, help='Unit test results output directory')

	args = parser.parse_args()

	emulator_exe = args.emulator
	unit_test_filename = args.unit_test_filename
	results_dir = args.results_dir

	if not os.path.exists(results_dir):
		os.makedirs(results_dir)

	# parse the json unit test file and run the emulator
	with open(unit_test_filename) as f:
		unit_test = json.load(f)

	base_path = os.path.dirname(os.path.abspath(unit_test_filename))

	for platform,tests in unit_test.items():
		test_cases = []
		for test in tests:
			print('Running unit test: %s' % (test['filename']))

			rom_filename = os.path.join(base_path, test['filename'])
			test_name = os.path.split(rom_filename)[1].split('.')[0]
			start_time = time.time()

			cmd = '"%s" -u -p %s -c "%s" -r %s' % (emulator_exe, test['abort_pc'], test['checksum'], rom_filename)
			ret = subprocess.run(cmd)

			if ret.returncode == 0:
				print('Test Passed')
			else:
				print('Test Failed')

			# generate test case
			test_cases.append(TestCase(test_name, elapsed_sec=(time.time() - start_time), status=ret.returncode))

		# generate test report
		ts = TestSuite(platform, test_cases)

		output_dir = os.path.join(results_dir, platform)
		if not os.path.exists(output_dir):
			os.makedirs(output_dir)

		with open(os.path.join(output_dir, 'results.xml'), 'w') as f:
			TestSuite.to_file(f, [ts], prettyprint=True)
			
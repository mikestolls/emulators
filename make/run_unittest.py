import json 
import datetime
import sys
import os
import shutil
import argparse
import subprocess

if __name__ == "__main__":
	
	parser = argparse.ArgumentParser(description='Run a unit test')
	parser.add_argument('--emulator', required=True, help='The emulator executable to run the unit test')
	parser.add_argument('--unit_test_filename', required=True, help='Unit test filename')

	args = parser.parse_args()

	emulator_exe = args.emulator
	unit_test_filename = args.unit_test_filename

	# parse the json unit test file and run the emulator
	with open(unit_test_filename) as f:
		unit_test = json.load(f)

	base_path = os.path.dirname(os.path.abspath(unit_test_filename))

	for test in unit_test:
		print('Running unit test: %s' % (test['filename']))

		rom_filename = os.path.join(base_path, test['filename'])

		try:
			cmd = '"%s" -u -p %s -c "%s" -r %s' % (emulator_exe, test['abort_pc'], test['checksum'], rom_filename)
			ret = subprocess.run(cmd)

			if ret.returncode == 0:
				print('Test Passed')
			else:
				print('Test Failed')

		except subprocess.CalledProcessError as e:
			pass
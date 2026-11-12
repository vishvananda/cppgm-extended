"""The common machine boundary accepts a non-x86 register-target representation."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class MachineBoundary(unittest.TestCase):
    def test_aarch64_representation_and_shared_call_facts(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / 'contract'
            compiler = shlex.split(os.environ.get('CPPGM_HOST_CXX', 'c++'))
            command = [*compiler, '-std=c++11', '-Wall', '-Werror',
                       '-I' + str(ROOT / 'dev/src'), str(ROOT / 'scripts/machine_boundary_contract.cpp'),
                       str(ROOT / 'dev/src/machine/call_arguments.cpp'), '-o', str(binary)]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()

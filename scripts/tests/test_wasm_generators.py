"""The shipped seeds must be reproducible by the supplied Perl generator."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
GENERATOR = ROOT / 'pa37/scripts/generate_tests.pl'


class WasmGenerators(unittest.TestCase):
    def test_shipped_seeds_match_both_and_individual_modes(self):
        for seed in (1, 305419896):
            with self.subTest(seed=seed), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                for kind in ('both', 'cfg', 'abi'):
                    output = root / kind
                    subprocess.run(['perl', str(GENERATOR), '--seed', str(seed), '--kind', kind,
                                    '--output', str(output)], check=True, capture_output=True)
                    for path in output.iterdir():
                        owner = 'control' if path.name.startswith('300-') else 'abi'
                        self.assertEqual(path.read_bytes(), (ROOT / 'pa37/tests' / owner / path.name).read_bytes())
                        self.assertEqual(path.read_bytes(), (root / 'both' / path.name).read_bytes())

    def test_invalid_inputs_do_not_generate_fixtures(self):
        with tempfile.TemporaryDirectory() as directory:
            for seed in ('-1', '4294967296', 'nan', '1.5'):
                result = subprocess.run(['perl', str(GENERATOR), '--seed', seed, '--output', directory],
                                        capture_output=True)
                self.assertNotEqual(result.returncode, 0)
            self.assertEqual(list(Path(directory).iterdir()), [])


if __name__ == '__main__':
    unittest.main()

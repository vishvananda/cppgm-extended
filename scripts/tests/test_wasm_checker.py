"""The real checker must retain failure checks and lower each source only once."""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

# Deliberately small protocol stub: resource enforcement itself is exercised by
# test_command_resources.py against the real native supervisor.
SUPERVISOR = r'''
import os, subprocess, sys
for line in sys.stdin:
    fields = line.rstrip('\n').split('\t')
    decode = lambda value: bytes.fromhex(value).decode()
    count = int(fields[8])
    command = [decode(x) for x in fields[9:9 + count]]
    index = 9 + count
    environment = os.environ.copy()
    for i in range(int(fields[index])):
        environment[decode(fields[index + 1 + i * 2])] = decode(fields[index + 2 + i * 2])
    with open(decode(fields[5]), 'rb') as inp, open(decode(fields[6]), 'wb') as out, open(decode(fields[7]), 'wb') as err:
        result = subprocess.run(command, stdin=inp, stdout=out, stderr=err, env=environment)
    print('exec-v1\t%d\texit' % result.returncode, flush=True)
'''
TOOL = r'''
from pathlib import Path
import json, os, sys
kind = Path(sys.argv[0]).name
args = sys.argv[1:]
with open(os.environ['CHECKER_COMMANDS'], 'a') as log:
    log.write(json.dumps([kind, args]) + '\n')
if kind == 'compiler':
    output = Path(args[args.index('-o') + 1])
    if '--emit-lowir' in args:
        output.write_text('target wasm32 1\nfunction @observe() -> i32 {\n block ^entry:\n return i32 17\n}\n')
    else:
        # A valid core module exporting observe() -> i32, returning 17.
        output.write_bytes(bytes.fromhex('0061736d010000000105016000017f03020100070b01076f62736572766500000a0601040041110b'))
elif kind == 'runtime':
    mode = os.environ.get('TRAP_MODE', 'ok')
    if mode == 'ok':
        print('17')
    else:
        message = 'all fuel consumed by WebAssembly' if mode == 'fuel' else 'wasm `unreachable` instruction executed'
        print('error: wasm trap: ' + message, file=sys.stderr)
        raise SystemExit(1)
'''


class WasmChecker(unittest.TestCase):
    def check(self, expect, mode='ok'):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, body in [('supervisor', SUPERVISOR), ('compiler', TOOL),
                               ('validator', TOOL), ('runtime', TOOL)]:
                path = root / name
                path.write_text('#!' + sys.executable + '\n' + body)
                path.chmod(0o755)
            fixture = root / 'case.t'
            fixture.write_text('target wasm32 1\nfunction @observe() -> i32 { block ^entry: return i32 17 }\n')
            fixture.with_suffix('.wasm-expect').write_text(json.dumps(expect))
            (root / 'extra.cpp').write_text('// supplied source unit\n')
            env = dict(os.environ, CPPGM_WASM_CPPGM=str(root / 'compiler'),
                       CPPGM_WASM_TOOLS=str(root / 'validator'), CPPGM_WASMTIME=str(root / 'runtime'),
                       CPPGM_TEST_SUPERVISOR=str(root / 'supervisor'),
                       CHECKER_COMMANDS=str(root / 'commands.jsonl'), TRAP_MODE=mode)
            result = subprocess.run(['perl', str(ROOT / 'scripts/check_wasm.pl'),
                                     str(root / 'compiler'), str(fixture)], env=env, cwd=root,
                                    capture_output=True, text=True, timeout=30)
            commands = [json.loads(x) for x in (root / 'commands.jsonl').read_text().splitlines()]
            return result, commands

    def test_lowering_is_shared_but_all_backend_optimization_levels_run(self):
        for units in ([], ['extra.cpp'], ['extra.cpp', 'extra.cpp']):
            with self.subTest(units=units):
                result, commands = self.check({'source': True, 'units': units,
                                               'calls': [{'export': 'observe', 'result': '17'}]})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                compiler = [args for tool, args in commands if tool == 'compiler']
                lowered = [args for args in compiler if '--emit-lowir' in args]
                self.assertEqual(len(lowered), 1 + bool(units))
                for level in range(4):
                    self.assertTrue(any('-O' + str(level) in args for args in compiler))

    def test_fuel_trap_is_distinct_from_an_unreachable_trap_or_return(self):
        expect = {'levels': [1], 'calls': [{'export': 'observe', 'trap': 'fuel', 'fuel': {'1': 1000}}]}
        result, commands = self.check(expect, 'fuel')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        runtime = [args for tool, args in commands if tool == 'runtime']
        self.assertTrue(runtime)
        self.assertTrue(all('memory-init-cow=n' in args and 'fuel=1000' in args for args in runtime))
        result, _ = self.check(expect, 'unreachable')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('expected fuel exhaustion', result.stderr)
        result, _ = self.check(expect, 'ok')
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('did not produce a Wasm trap', result.stderr)

    def test_ordinary_traps_and_wrong_results_still_fail_correctly(self):
        result, _ = self.check({'levels': [0], 'calls': [{'export': 'observe', 'trap': True}]}, 'unreachable')
        self.assertEqual(result.returncode, 0, result.stderr)
        result, _ = self.check({'levels': [0], 'calls': [{'export': 'observe', 'result': '18'}]})
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("expected 18, got '17'", result.stderr)


if __name__ == '__main__':
    unittest.main()

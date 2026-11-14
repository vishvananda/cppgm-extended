#!/usr/bin/env python3
"""Exercise optional Wasm embedding controls against the real built tools."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
SOURCE = '''extern "C" int host_ready();
int initialized = host_ready() + 5;
thread_local int tls_value = 3;
thread_local int tls_lazy = host_ready();
extern "C" int observe() { asm volatile("" ::: "memory"); return initialized; }
extern "C" int read_tls() { return ++tls_value + tls_lazy; }
extern "C" int callback(int value) { return value + 2; }
extern "C" unsigned callback_address() { return reinterpret_cast<unsigned>(&callback); }
'''
CHECK = r'''
const fs = require('fs');
(async () => {
  let ready = false, calls = 0;
  const module = await WebAssembly.compile(fs.readFileSync(process.argv[1]));
  const instance = await WebAssembly.instantiate(module, {env:{host_ready:()=>{
    if (!ready) throw Error('constructor ran before host initialization');
    calls++; return 12;
  }}});
  const e = instance.exports;
  if (calls || e.observe() !== 0) throw Error('constructors were not deferred');
  ready = true;
  e.__wasm_call_ctors();
  if (calls !== 1 || e.observe() !== 17) throw Error('constructor result differs');
  if (e.__indirect_function_table.get(e.callback_address())(15) !== 17)
    throw Error('exported table cannot call the actual function');
  if (e.read_tls() !== 16 || e.read_tls() !== 17 || calls !== 2)
    throw Error('TLS initialization or repeated access differs');
  const other = await WebAssembly.instantiate(module, {env:{host_ready:()=>12}});
  if (other.exports.observe() !== 0 || other.exports.read_tls() !== 16)
    throw Error('TLS storage leaked between module instances');
  console.log(JSON.stringify({passed:true, constructors:calls, result:e.observe()}));
})().catch(error => { console.error(error); process.exitCode=1; });
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler', type=Path, default=ROOT / 'dev/cppgm++')
    parser.add_argument('--backend', type=Path, default=ROOT / 'dev/lowir2native')
    parser.add_argument('--node', default='node')
    args = parser.parse_args()
    compiler, backend = str(args.compiler.resolve()), str(args.backend.resolve())
    records = []

    def run(command, success=True):
        result = subprocess.run(command, capture_output=True, text=True, timeout=90)
        records.append(dict(command=command, exit=result.returncode, stdout=result.stdout, stderr=result.stderr))
        if success and result.returncode:
            raise RuntimeError(f'{command}: {result.stdout}{result.stderr}')
        if not success and (result.returncode != 1 or 'ERROR:' not in result.stderr):
            raise RuntimeError(f'expected a compiler diagnostic: {command}: {result.stderr}')
        return result

    with tempfile.TemporaryDirectory(prefix='cppgm-wasm-embedding-') as directory:
        root = Path(directory)
        source, lowir = root / 'input.cpp', root / 'input.lowir'
        source.write_text(SOURCE)
        common = ['--target=wasm32-unknown-unknown', '-nostdlib']
        exports = ['--export', 'observe', '--export', 'callback_address', '--export', 'read_tls',
                   '--export-table', '--export-ctors']
        run([compiler, '--target=wasm32-unknown-unknown', '--emit-lowir', str(source), '-o', str(lowir)])
        for level in [0, 3]:
            for name, tool, input in [('source', compiler, source), ('lowir', backend, lowir)]:
                output = root / f'{name}-{level}.wasm'
                run([tool, *common, f'-O{level}', '--import-function=host_ready', *exports,
                     str(input), '-o', str(output)])
                run([args.node, '-e', CHECK, str(output)])
                run([tool, *common, f'-O{level}', *exports, str(input), '-o', str(output)], success=False)
                run([tool, '--target=x86_64-linux-gnu', '--export-table', str(input), '-o', str(output)], success=False)
        for flag in ['--import-function', '--import-function=']:
            run([compiler, *common, str(source), '-o', str(root / 'bad.wasm'), flag], success=False)
        empty = root / 'empty.cpp'
        empty.write_text('extern "C" int observe(){return 17;}\n')
        output = root / 'empty.wasm'
        run([compiler, *common, '--export', 'observe', '--export-ctors', str(empty), '-o', str(output)])
        run([args.node, '-e', 'const e=new WebAssembly.Instance(new WebAssembly.Module(require("fs").readFileSync(process.argv[1]))).exports;e.__wasm_call_ctors();if(e.observe()!==17)throw Error("wrong empty constructor result");', str(output)])
    print(json.dumps(dict(passed=True, commands=records), indent=2))


if __name__ == '__main__':
    main()

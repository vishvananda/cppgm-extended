#!/usr/bin/env python3
"""Build and exercise the nine reference tools as WASI command modules."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tarfile
import time
import urllib.request

from experimental import wasi_inception as frozen

TOOLS = ['abimangle', 'pptoken', 'posttoken', 'ppexpr', 'preproc', 'cppgm++',
         'lowir', 'lowir2native', 'lowiropt']


def checksum(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def export(args):
    args.reference_tools = TOOLS
    args.x86_headers = True
    args.optimization = 3
    version = (args.sdk / 'VERSION').read_text()
    if not all(line in version.splitlines() for line in
               ['34.0', 'wasi-libc: 2e6fb9d8ee0c', 'llvm: 895aa2c896ad']):
        raise RuntimeError('update the release notices for this WASI SDK revision')
    frozen.prepare(args)
    package = args.directory.resolve()
    inputs = package / 'inputs'
    manifest = json.loads((package / 'manifest.json').read_text())
    frozen.verify(package, manifest)
    sources = sorted({s for group in manifest['tool_sources'].values() for s in group})
    output = inputs / 'reference-wasm'
    output.mkdir()
    observations = []

    def compile(source):
        target = output / frozen.object_name(source)
        command = [str(package / 'native-compiler'), *manifest['flags'], '-c', source,
                   '-o', str(target)]
        start = time.monotonic()
        with target.with_suffix('.log').open('wb') as log:
            subprocess.run(command, cwd=inputs, stdout=log, stderr=log, check=True)
        return dict(source=source, seconds=time.monotonic() - start, sha256=checksum(target))

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        observations.extend(pool.map(compile, sources))
    (output / 'smoke.cpp').write_text('int main() { return 0; }\n')
    binaries = {}
    for name in TOOLS:
        module = output / (name + '.wasm')
        objects = [str(output / frozen.object_name(s)) for s in manifest['tool_sources'][name]]
        with (output / (name + '.link.log')).open('wb') as log:
            subprocess.run([str(package / 'native-compiler'), *manifest['flags'], *objects,
                            *manifest['link_flags'], '-o', str(module)], cwd=inputs,
                           stdout=log, stderr=log, check=True)
        runner = [str(args.wasmtime.resolve()), 'run', '-W', 'exceptions=y', '-W',
                  'max-wasm-stack=8388608', '--dir', str(inputs) + '::.', str(module)]
        smoke = ['-o', 'reference-wasm/smoke.tokens', 'reference-wasm/smoke.cpp'] if name == 'preproc' else ['--help']
        if name in {'pptoken', 'posttoken', 'ppexpr'}:
            smoke = []
        subprocess.run(runner + smoke, input='1\n', text=True, cwd=inputs,
                       stdout=subprocess.DEVNULL, check=True, timeout=120)
        binaries[name + '.wasm'] = checksum(module)
    # Check useful work across tools, not only startup and --help.
    def run(name, *arguments):
        subprocess.run([str(args.wasmtime.resolve()), 'run', '-W', 'exceptions=y', '-W',
                        'max-wasm-stack=8388608', '--dir', str(inputs) + '::.',
                        str(output / (name + '.wasm')), *arguments], cwd=inputs, check=True, timeout=120)
    run('lowir', '--exercise', 'sum', '-o', 'reference-wasm/exercise.lowir')
    run('lowiropt', '-O3', 'reference-wasm/exercise.lowir', '-o', 'reference-wasm/optimized.lowir')
    shutil.copyfile(frozen.REPO / 'pa8/tests/behavior/100-sum.t', output / 'entry.lowir')
    run('lowir2native', '-O3', '-o', 'reference-wasm/sum.program',
        'reference-wasm/optimized.lowir', 'reference-wasm/entry.lowir')
    program = output / 'sum.program'
    program.chmod(0o755)  # WASI does not have chmod; the native host executes this ELF.
    subprocess.run([str(program)], check=True, timeout=30)
    shutil.copyfile(frozen.REPO / 'pa9/tests/abi/100-c-linkage-function.t', output / 'abi.facts')
    run('abimangle', '-o', 'reference-wasm/abi.names', 'reference-wasm/abi.facts')
    if (output / 'abi.names').read_text().strip() != 'puts':
        raise RuntimeError('WASI ABI tool emitted an unexpected C-linkage name')
    # LowIR text uses long-double formatting even for binary32/binary64 values.
    # wasi-libc's default formatting stub traps unless its optional library is linked.
    (output / 'text.cpp').write_text('float f = 1.25f; double g(double x) { return x + 0.1; }\n')
    for target, flags in [('wasm', manifest['flags']), ('x86', manifest['x86_flags'])]:
        reference = output / ('text.' + target + '.native.lowir')
        actual = output / ('text.' + target + '.wasi.lowir')
        subprocess.run([str(package / 'native-compiler'), *flags, '--emit-lowir',
                        'reference-wasm/text.cpp', '-o', str(reference)], cwd=inputs, check=True)
        run('cppgm++', *flags, '--emit-lowir', 'reference-wasm/text.cpp', '-o', str(actual.relative_to(inputs)))
        frozen.compare(reference, actual)
        run('lowiropt', '-O0', str(actual.relative_to(inputs)), '-o', 'reference-wasm/text.optimized.lowir')
    # The full compiler package's floating cases exercise the wide constant evaluator.
    for target, flags in [('wasm', manifest['flags']), ('x86', manifest['x86_flags'])]:
        reference = output / ('floating.' + target + '.native.o')
        actual = output / ('floating.' + target + '.wasi.o')
        subprocess.run([str(package / 'native-compiler'), *flags, '-O3', '-c', 'checks/floating.cpp',
                        '-o', str(reference)], cwd=inputs, check=True)
        run('cppgm++', *flags, '-O3', '-c', 'checks/floating.cpp', '-o', str(actual.relative_to(inputs)))
        frozen.compare(reference, actual)
    frozen.verify(package, manifest)
    record = dict(schema=1, platform='wasm32-wasip1', source_sha=manifest['revision'],
                  dirty=manifest['dirty'], sdk_version=manifest['sdk_version'],
                  compiler_sha256=manifest['native_sha256'], binaries=binaries,
                  compilation=observations)
    frozen.save(package / 'wasm-reference-provenance.json', record)
    licenses = package / 'licenses'
    licenses.mkdir()
    notices = Path(__file__).with_name('wasm_release_licenses.json')
    for name, row in json.loads(notices.read_text()).items():
        target = licenses / name
        with urllib.request.urlopen(row['source'], timeout=60) as response:
            target.write_bytes(response.read())
        if checksum(target) != row['sha256']:
            raise RuntimeError('release notice checksum mismatch: ' + name)
    shutil.copyfile(notices, licenses / 'sources.json')
    readme = package / 'README.txt'
    readme.write_text('''WASI reference tools\n\nThese nine command modules were built by cppgm at O3. Statistics and\npersistent POSIX batch-runner support are disabled in this host profile.\nThe SDK libraries are prebuilt WASI SDK dependencies, not built from C here.\n\nThe frozen filesystem includes compiler source, target headers and SDK inputs.\nUnpack it into a new directory, then run a module with Wasmtime 50 or newer:\n\n  python3 unpack_wasi_inputs.py . inputs\n  cd inputs\n  wasmtime run -W exceptions=y -W max-wasm-stack=8388608 --dir . ../bin/cppgm++.wasm --help\n\nUse --target=wasm32-wasip1 --sysroot=sysroot -resource-dir=resource for WASI\nsource. The default code target is x86_64 Linux; its frozen header paths\nare relative to inputs. External native host compiler/linker subprocesses\nare unavailable in WASI. Compilation and the internal linkers remain local.\n\nmanifest.json records input hashes and canonical flags.\nwasm-reference-provenance.json records the source and compiler identity.\nLICENSE and NOTICE cover cppgm; licenses/ covers the bundled SDK and headers.\n''')
    # Include target headers and SDK inputs: these tools never consult a machine's
    # native include directories. Browser consumers can use the same frozen VFS.
    args.bundle.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(args.bundle, 'w:gz') as archive:
        for name in TOOLS:
            archive.add(output / (name + '.wasm'), arcname='bin/' + name + '.wasm')
        for name in ['manifest.json', 'manifest.sha256', 'bundle.bin', 'wasm-reference-provenance.json']:
            archive.add(package / name, arcname=name)
        for name in ['LICENSE', 'NOTICE']:
            archive.add(inputs / name, arcname=name)
        archive.add(licenses, arcname='licenses')
        archive.add(readme, arcname='README.txt')
        archive.add(Path(__file__).with_name('unpack_wasi_inputs.py'), arcname='unpack_wasi_inputs.py')
    print(f'WASI reference tools: {len(binaries)}/9 passed; {args.bundle}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory', type=Path, required=True, help='new, disposable build directory')
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--compiler', type=Path, required=True)
    parser.add_argument('--wasmtime', type=Path, required=True)
    parser.add_argument('--shim', type=Path, required=True)
    parser.add_argument('--bundle', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=2)
    export(parser.parse_args())

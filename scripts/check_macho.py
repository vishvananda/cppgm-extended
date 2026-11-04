#!/usr/bin/env python3
"""Check optional x86-64 Mach-O output; execute and host-link it on Intel macOS."""
import argparse
from pathlib import Path
import platform
import shutil
import struct
import subprocess
import sys
import tempfile


def run(command, expected=0):
    result = subprocess.run([str(x) for x in command], text=True, capture_output=True, timeout=180)
    if result.returncode != expected:
        raise RuntimeError(f'{command}: exit {result.returncode}, expected {expected}\n'
                           f'{result.stdout}{result.stderr}')
    return result.stdout


def macho(path, filetype):
    data = path.read_bytes()
    magic, cpu, _, kind, count, size, _, _ = struct.unpack_from('<8I', data)
    assert (magic, cpu, kind) == (0xfeedfacf, 0x01000007, filetype), path
    at = 32
    for _ in range(count):
        _, length = struct.unpack_from('<II', data, at)
        assert length >= 8 and at + length <= 32 + size <= len(data), path
        at += length
    assert at == 32 + size, path


def inspect(path):
    readobj = shutil.which('llvm-readobj')
    if readobj:
        run([readobj, '--file-headers', '--sections', '--relocations', '--symbols', path])
    dwarfdump = shutil.which('llvm-dwarfdump')
    if dwarfdump:
        run([dwarfdump, '--verify', path])
    lld = shutil.which('ld64.lld')
    if lld:
        run([lld, '-dylib', '-undefined', 'dynamic_lookup', '-arch', 'x86_64',
             '-platform_version', 'macos', '10.15', '10.15', path, '-o', path.with_suffix('.dylib')])


def image(args, work, native):
    output = work / 'standalone'
    run([args.lowir2native, '--target', 'macos', args.fixtures / '100-entry.lowir', '-o', output])
    macho(output, 2)
    if native:
        run([output], 37)


def objects(args, work, native):
    for level in range(4):
        inputs = []
        for name in ['100-main', '100-data']:
            output = work / f'{name}-{level}.obj'
            run([args.compiler, '--target=macos', f'-O{level}', '-c',
                 args.fixtures / f'{name}.cpp', '-o', output])
            macho(output, 1)
            inputs.append(output)
        output = work / f'linked-{level}'
        run([args.compiler, '--target=macos', *inputs, '-o', output])
        macho(output, 2)
        if native:
            run([output])
    # Bounds and object-format errors must be diagnostics, never crashes.
    source = inputs[0].read_bytes()
    for number, broken in enumerate([source[:20], source[:60], source[:16] + b'\xff' * 16 + source[32:]]):
        path = work / f'truncated-{number}.o'
        path.write_bytes(broken)
        run([args.compiler, '--target=macos', path, '-o', work / 'bad'], 1)


def interop(args, work, native):
    for level in range(4):
        output = work / f'generated-{level}.o'
        run([args.compiler, '--target=macos', f'-O{level}', '-g', '-c',
             args.fixtures / '200-generated.cpp', '-o', output])
        macho(output, 1)
        inspect(output)
        if native:
            executable = work / f'interop-{level}'
            run([args.host_cxx, '-std=c++11', '-pthread', '-g', args.fixtures / '200-host.cpp',
                 output, '-o', executable])
            run([executable])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=['image', 'object', 'interop'], required=True)
    parser.add_argument('--fixtures', type=Path, required=True)
    parser.add_argument('--compiler', type=Path, default=Path('dev/cppgm++'))
    parser.add_argument('--lowir2native', type=Path, default=Path('dev/lowir2native'))
    parser.add_argument('--host-cxx', default='clang++')
    args = parser.parse_args()
    args.compiler = args.compiler.resolve()
    args.lowir2native = args.lowir2native.resolve()
    args.fixtures = args.fixtures.resolve()
    native = sys.platform == 'darwin' and platform.machine() == 'x86_64'
    with tempfile.TemporaryDirectory(prefix='cppgm-macho-') as temporary:
        modes = {'image': image, 'object': objects, 'interop': interop}
        modes[args.mode](args, Path(temporary), native)
    print(f'Mach-O {args.mode}: PASS ({"native Intel macOS" if native else "cross emission"})')


if __name__ == '__main__':
    main()

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

FIXTURES = {'image': ['100-entry.lowir'],
            'object': ['100-main.cpp', '100-data.cpp'],
            'host': ['100-handler-rejoin.cpp'],
            'runtime': ['200-host-thread-local-reference-temporary.t.1',
                        '200-host-thread-local-reference-lifetime.t.1'],
            'interop': ['200-generated.cpp', '200-host.cpp', '200-host-tls.cpp']}


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


def inspect(path, extra=()):
    readobj = shutil.which('llvm-readobj')
    if readobj:
        run([readobj, '--file-headers', '--sections', '--relocations', '--symbols', path])
    dwarfdump = shutil.which('llvm-dwarfdump')
    if dwarfdump:
        run([dwarfdump, '--verify', path])
    lld = shutil.which('ld64.lld')
    if lld and extra is not None:
        run([lld, '-dylib', '-undefined', 'dynamic_lookup', '-arch', 'x86_64',
             '-platform_version', 'macos', '10.15', '10.15', path, *extra,
             '-o', path.with_suffix('.dylib')])


def image(args, work, native):
    output = work / 'standalone'
    run([args.lowir2native, '--target', 'macos', args.fixtures / FIXTURES['image'][0], '-o', output])
    macho(output, 2)
    if native:
        run([output], 37)


def objects(args, work, native):
    for level in range(4):
        inputs = []
        for name in FIXTURES['object']:
            output = work / f'{Path(name).stem}-{level}.obj'
            run([args.compiler, '--target=macos', f'-O{level}', '-c',
                 args.fixtures / name, '-o', output])
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


def host_objects(args, work, native):
    for name in FIXTURES[args.mode]:
        for level in range(4):
            output = work / f'{name}-{level}.o'
            run([args.compiler, '--target=macos', f'-O{level}', '-g', '-c',
                 args.fixtures / name, '-o', output])
            macho(output, 1)
            inspect(output)
            if native:
                executable = output.with_suffix('')
                run([args.host_cxx, '-pthread', output, '-o', executable])
                run([executable])


def interop(args, work, native):
    clang = args.host_cxx if native else shutil.which('clang++')
    helper = work / 'host-tls.o'
    if clang:
        run([clang, '--target=x86_64-apple-macos10.15', '-std=c++11', '-c',
             args.fixtures / FIXTURES['interop'][2], '-o', helper])
    for level in range(4):
        output = work / f'generated-{level}.o'
        run([args.compiler, '--target=macos', f'-O{level}', '-g', '-c',
             args.fixtures / FIXTURES['interop'][0], '-o', output])
        macho(output, 1)
        inspect(output, [helper] if clang else None)
        if native:
            executable = work / f'interop-{level}'
            run([args.host_cxx, '-std=c++11', '-pthread', '-g', args.fixtures / FIXTURES['interop'][1],
                 args.fixtures / FIXTURES['interop'][2], output, '-o', executable])
            run([executable])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mode', choices=['image', 'object', 'host', 'runtime', 'interop'])
    parser.add_argument('--fixtures', type=Path)
    parser.add_argument('--list', action='store_true', help='list selected fixture inputs without running them')
    parser.add_argument('roots', nargs='*', type=Path)
    parser.add_argument('--compiler', type=Path, default=Path('dev/cppgm++'))
    parser.add_argument('--lowir2native', type=Path, default=Path('dev/lowir2native'))
    parser.add_argument('--host-cxx', default='clang++')
    args = parser.parse_args()
    if args.list:
        for root in args.roots + ([args.fixtures] if args.fixtures else []):
            for mode, names in FIXTURES.items():
                if args.mode and mode != args.mode:
                    continue
                for name in names:
                    if (root / name).is_file():
                        print(root / name)
        return
    if not args.mode or not args.fixtures:
        parser.error('--mode and --fixtures are required unless listing fixtures')
    args.compiler = args.compiler.resolve()
    args.lowir2native = args.lowir2native.resolve()
    args.fixtures = args.fixtures.resolve()
    native = sys.platform == 'darwin' and platform.machine() == 'x86_64'
    with tempfile.TemporaryDirectory(prefix='cppgm-macho-') as temporary:
        modes = {'image': image, 'object': objects, 'host': host_objects,
                 'runtime': host_objects, 'interop': interop}
        modes[args.mode](args, Path(temporary), native)
    print(f'Mach-O {args.mode}: PASS ({"native Intel macOS" if native else "cross emission"})')


if __name__ == '__main__':
    main()

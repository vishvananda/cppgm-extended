#!/usr/bin/env python3
"""Build unmodified pinned Godot and compare checked headless workloads."""
import argparse
import json
import math
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys

import benchmark_hosts
import benchmark_timing as timing
import index_cpp_benchmarks as measure
import index_cpp_ci as common
import public_benchmarks
import simdjson_ci as paired

ROOT = Path(__file__).resolve().parents[1]
COMMIT = 'ed1daf0bf001b61586d9930840f2f1394092c079'  # 4.7.2-stable
UPSTREAM = 'https://github.com/godotengine/godot.git'
LABELS = ('cppgm', 'gcc', 'clang')
WORKLOADS = ('dispatch', 'arrays', 'strings', 'geometry', 'scene')
BUILD_FLAGS = [
    'platform=linuxbsd', 'target=template_release', 'arch=x86_64', 'optimize=speed',
    'debug_symbols=no', 'lto=none', 'use_static_cpp=no', 'disable_path_overrides=no',
    'x11=no', 'wayland=no', 'vulkan=no', 'opengl3=no', 'alsa=no', 'pulseaudio=no',
    'dbus=no', 'speechd=no', 'fontconfig=no', 'udev=no', 'accesskit=no',
    'compiledb=yes', 'verbose=yes',
]


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def source_check(source):
    revision = subprocess.check_output(['git', '-C', source, 'rev-parse', 'HEAD'], text=True).strip()
    dirty = subprocess.check_output(['git', '-C', source, 'status', '--porcelain'], text=True)
    if revision != COMMIT or dirty:
        raise ValueError('Godot must be the clean pinned upstream checkout')


def fetch(source):
    source.mkdir(parents=True)
    for args in [('init', '-q'), ('remote', 'add', 'origin', UPSTREAM),
                 ('fetch', '-q', '--depth', '1', 'origin', COMMIT),
                 ('checkout', '-q', '--detach', 'FETCH_HEAD')]:
        subprocess.run(['git', '-C', source, *args], check=True)


def expected(name, count):
    if name == 'dispatch':
        return sum((i, -i, i * 3, -i * 2)[i & 3] for i in range(count))
    if name == 'arrays':
        return sum((i * 17) & 65535 for i in range(count))
    if name == 'strings':
        return sum(len(f'compiler:{i & 255}:{i & 15}') + 8 for i in range(count))
    if name == 'geometry':
        return sum(3 * (i & 15) - (i & 7) - 2 * (i & 31) for i in range(count))
    if name == 'scene':
        return sum((i & 31) + (i & 15) + 5 for i in range(count))
    raise ValueError('unknown Godot workload: ' + name)


def parse_output(text, name, count, repeats):
    lines = [line.removeprefix('CPPGM_GODOT ') for line in text.splitlines()
             if line.startswith('CPPGM_GODOT ')]
    if len(lines) != 1:
        raise ValueError('missing or duplicate Godot result')
    result = json.loads(lines[0])
    if (result.get('workload'), result.get('iterations'), result.get('repeats'), result.get('checksum')) != (
            name, count, repeats, expected(name, count) * repeats):
        raise ValueError('Godot workload identity or checksum mismatch')
    elapsed = result.get('elapsed_usec')
    if not isinstance(elapsed, (int, float)) or not math.isfinite(elapsed) or elapsed <= 0:
        raise ValueError('invalid Godot workload time')
    return result


def build(args, directory, source, compilers, cpus):
    builds, observations = {}, {}
    launcher = directory / 'cppgm-launcher'
    launcher.write_text('#!/bin/sh\nhost=$1\nshift\nfor arg do\n'
                        '  if [ "$arg" = -c ]; then exec ' + shlex.quote(str(compilers['cppgm'])) +
                        ' --full-asm "$@"; fi\ndone\nexec "$host" "$@"\n')
    launcher.chmod(0o755)
    for label in LABELS:
        tree = directory / ('build-' + label)
        subprocess.run(['git', 'clone', '-q', '--no-hardlinks', str(source), str(tree)], check=True)
        command = [args.scons, *BUILD_FLAGS, 'CC=' + str(common.compiler_path(args.cc)),
                   'CXX=' + str(compilers['clang'] if label == 'clang' else compilers['gcc']),
                   'LINK=' + str(compilers['gcc']), '-j' + str(args.jobs)]
        if label == 'cppgm':
            command.append('cpp_compiler_launcher=' + str(launcher))
        # Enter the private build tree in a child; no shell interpretation of flags.
        invocation = [sys.executable, '-c',
                      'import os,sys;os.chdir(sys.argv[1]);os.execv(sys.argv[2],sys.argv[2:])',
                      str(tree), *command]
        row = measure.run(invocation, directory / ('build-' + label), cpus, args.build_timeout, counters=False)
        timing.retain(directory / 'compile-observations.jsonl', dict(row, label=label))
        if row['status'] != 0:
            for stream in ('stdout', 'stderr'):
                text = Path(row[stream]).read_text(errors='replace')
                print(f'Godot {label} {stream} (tail):\n{text[-8000:]}', file=sys.stderr)
            raise ValueError('Godot full build failed: ' + label)
        observations[label] = row
        executable = tree / ('bin/godot.linuxbsd.template_release.x86_64' + ('.llvm' if label == 'clang' else ''))
        builds[label] = executable.resolve(strict=True)
        database = json.loads((tree / 'compile_commands.json').read_text())
        save(directory / ('commands-' + label + '.json'), database)
    return observations, builds


def runtime(args, directory, binaries):
    project = directory / 'project'
    project.mkdir()
    for name in ('project.godot', 'workload.gd'):
        shutil.copy2(ROOT / 'benchmarks/godot' / name, project / name)
    cache = project / '.godot'
    cache.mkdir()
    (cache / 'global_script_class_cache.cfg').write_text('list=[]\n')
    hashes = {label: timing.sha(path) for label, path in binaries.items()}
    results, checks = {}, {}
    serial = 0
    for name in WORKLOADS:
        count, repeats = 100000, 1

        def one(label, phase, block, position):
            nonlocal serial
            prefix = directory / f'runtime-{serial:04}-{label}'
            serial += 1
            row = measure.run([binaries[label], '--headless', '--path', project, '--script', 'workload.gd',
                               '--', name, str(count), str(repeats)], prefix, args.runtime_cpu,
                              args.runtime_timeout, counters=False)
            row.update(label=label, workload=name, phase=phase, block=block, position=position)
            try:
                if row['status'] != 0:
                    raise ValueError('Godot process failed: ' + str(prefix))
                stdout = Path(row['stdout']).read_text()
                stderr = Path(row['stderr']).read_text()
                if re.search(r'(?m)^(?:SCRIPT ERROR|ERROR):', stdout + '\n' + stderr):
                    raise ValueError('Godot reported an error: ' + str(prefix))
                record = parse_output(stdout, name, count, repeats)
                row.update(checksum=record['checksum'], measured_wall=record['elapsed_usec'] / 1e6)
            finally:
                timing.retain(directory / 'runtime-observations.jsonl', row)
            return row

        checks[name] = {label: one(label, 'correctness', 0, 0)['checksum'] for label in LABELS}
        pilot = one('gcc', 'pilot', 0, 0)
        repeats = max(1, math.ceil(args.seconds / pilot['measured_wall']))
        results[name] = {ref: paired.paired(one, ref, args.blocks, args.aa_blocks, ('cpu', 'measured_wall'))
                         for ref in ('gcc', 'clang')}
    if hashes != {label: timing.sha(path) for label, path in binaries.items()}:
        raise ValueError('a measured Godot executable changed')
    return results, checks, hashes


def validate(document):
    if document.get('status') != 'complete' or document.get('source_commit') != COMMIT:
        raise ValueError('Godot results are incomplete or from another source')
    if set(document['runtime']) != set(WORKLOADS) or set(document['correctness']) != set(WORKLOADS):
        raise ValueError('missing Godot workloads')
    for name in WORKLOADS:
        checks = document['correctness'][name]
        if set(checks) != set(LABELS) or set(checks.values()) != {expected(name, 100000)}:
            raise ValueError('invalid Godot checksum evidence')
        for ref in ('gcc', 'clang'):
            for key in ('cpu', 'measured_wall'):
                interval = document['runtime'][name][ref][key]
                values = [interval, interval['calibration_before'], interval['calibration_after']]
                for value in values:
                    if not value['blocks'] or not math.isfinite(value['speed']) or value['speed'] <= 0:
                        raise ValueError('invalid Godot timing evidence')
    if set(document['compilation']) != set(LABELS):
        raise ValueError('missing full build evidence')
    for row in document['compilation'].values():
        if row['status'] != 0 or any(not math.isfinite(row[k]) or row[k] <= 0 for k in ('cpu', 'wall')):
            raise ValueError('failed or invalid full build')


def report(document):
    text = '# Godot headless benchmarks\n\n'
    if document['status'] != 'complete':
        return text + '**Incomplete:** ' + document.get('error', 'running') + '\n'
    metadata = document['metadata']
    text += (f"Compiler: `{metadata['revision']}` · CPU: {metadata['cpu']}\n\n"
             f'Unmodified Godot 4.7.2 (`{COMMIT}`), full headless release template, C++17 `-O3`, '
             'all default engine modules, 3D and computed GDScript dispatch retained. '
             'C files and final linking use GCC for all three builds.\n\n'
             'Full-build times below are one clean build per compiler, including SCons and linking. '
             'They are not paired microbenchmark measurements.\n\n'
             '| Compiler | Build CPU seconds | Build wall seconds |\n|---|---:|---:|\n')
    for label in LABELS:
        row = document['compilation'][label]
        text += f"| {label} | {row['cpu']:.2f} | {row['wall']:.2f} |\n"
    text += ('\nRuntime ratios are **reference time / cppgm time: higher is better**. '
             'Work wall time excludes engine startup; process CPU time includes it. '
             'Fixed checked GDScript workloads exercise dispatch, arrays, strings, geometry and scene transforms; '
             'these are not rendering or game frame-rate measurements. Every checksum must match an independent calculation. '
             'Repeated work exceeds the configured pilot duration. Paired ABBA/BAAB blocks include A/A calibration before and after.\n\n'
             '| Workload | vs GCC CPU | vs Clang CPU | vs GCC work wall | vs Clang work wall |\n|---|---:|---:|---:|---:|\n')
    warnings = []
    for name in WORKLOADS:
        values = document['runtime'][name]
        text += '| ' + name + ' | ' + ' | '.join(f"{values[ref][key]['speed']:.3f}×"
                    for key in ('cpu', 'measured_wall') for ref in ('gcc', 'clang')) + ' |\n'
        for ref in ('gcc', 'clang'):
            for key in ('cpu', 'measured_wall'):
                for phase in ('before', 'after'):
                    speed = values[ref][key]['calibration_' + phase]['speed']
                    if abs(speed - 1) > .02:
                        warnings.append(f'{name} {ref} {key} {phase}: {speed:.3f}×')
    text += '\nA/A variation above 2%: ' + ('; '.join(warnings) if warnings else 'none') + '. Every observation is retained.\n'
    text += '\n## Toolchains\n\n' + '\n'.join(f'- {label}: {text.splitlines()[0]}'
                    for label, text in metadata['toolchains'].items()) + '\n'
    return text + ('\n[Results](results.json) · [Provenance and build flags](metadata.json) · '
                   '[Full-build observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)\n')


def run(args):
    directory = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=False)
    document = dict(status='running', source_commit=COMMIT)
    try:
        compiler = directory / 'compiler'
        shutil.copy2(args.compiler.resolve(strict=True), compiler)
        compiler.chmod(0o555)
        compilers = dict(cppgm=compiler, gcc=common.compiler_path(args.gcc), clang=common.compiler_path(args.clang))
        cpus = ','.join(map(str, sorted(os.sched_getaffinity(0))))
        hashes = {label: timing.sha(path) for label, path in compilers.items()}
        metadata = dict(revision=subprocess.check_output(['git', '-C', ROOT, 'rev-parse', 'HEAD'], text=True).strip(),
                        dirty=bool(subprocess.check_output(['git', '-C', ROOT, 'status', '--porcelain'])),
                        cpu=public_benchmarks.cpu_model(), cpus=cpus, runtime_cpu=args.runtime_cpu,
                        flags=BUILD_FLAGS, compiler_hashes=hashes,
                        workload_hashes={name: timing.sha(ROOT / 'benchmarks/godot' / name)
                                         for name in ('project.godot', 'workload.gd')},
                        toolchains={label: subprocess.check_output([str(path), '--version'], text=True).strip()
                                    for label, path in compilers.items()})
        save(directory / 'metadata.json', metadata)
        source = args.source.resolve() if args.source else directory / 'source'
        if args.source is None:
            fetch(source)
        source_check(source)
        compilation, binaries = build(args, directory, source, compilers, cpus)
        measurements, correctness, binary_hashes = runtime(args, directory, binaries)
        source_check(source)
        if hashes != {label: timing.sha(path) for label, path in compilers.items()}:
            raise ValueError('a compiler changed during measurement')
        document.update(status='complete', metadata=metadata, compilation=compilation,
                        runtime=measurements, correctness=correctness, binary_hashes=binary_hashes)
        validate(document)
    except BaseException as error:
        document.update(status='failed', error=repr(error))
        raise
    finally:
        save(directory / 'results.json', document)
        (directory / 'details.md').write_text(report(document))


def publish(args):
    document = json.loads((args.artifact / 'results.json').read_text())
    validate(document)
    metadata = document['metadata']
    if not re.fullmatch('[0-9a-f]{40}', args.revision) or metadata['revision'] != args.revision or metadata['dirty']:
        raise ValueError('results do not describe the clean measured revision')
    files = ('results.json', 'metadata.json', 'compile-observations.jsonl', 'runtime-observations.jsonl')
    payloads = {name: (args.artifact / name).read_bytes() for name in files}
    if json.loads(payloads['metadata.json']) != metadata:
        raise ValueError('provenance differs from the measured results')
    platform = benchmark_hosts.hosted_platform(metadata['cpu'])
    for dest in (args.target / 'godot' / platform, args.target / 'godot/history' / args.revision / platform):
        dest.mkdir(parents=True, exist_ok=True)
        for name, value in payloads.items():
            (dest / name).write_bytes(value)
        (dest / 'report.md').write_text(report(document) + '\n[CI run and artifacts](' + args.run_url + ')\n')
    benchmark_hosts.library_index(args.target / 'godot', 'Godot benchmark hosts')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--compiler', type=Path, default=ROOT / 'dev/cppgm++')
    runner.add_argument('--source', type=Path)
    runner.add_argument('--output', type=Path, required=True)
    runner.add_argument('--gcc', default='g++')
    runner.add_argument('--cc', default='gcc')
    runner.add_argument('--clang', default='clang++')
    runner.add_argument('--scons', default=shutil.which('scons'))
    runner.add_argument('--jobs', type=int, default=len(os.sched_getaffinity(0)))
    runner.add_argument('--runtime-cpu', default=str(min(os.sched_getaffinity(0))))
    runner.add_argument('--blocks', type=int, default=3)
    runner.add_argument('--aa-blocks', type=int, default=1)
    runner.add_argument('--seconds', type=float, default=.5)
    runner.add_argument('--build-timeout', type=int, default=7200)
    runner.add_argument('--runtime-timeout', type=int, default=120)
    publisher = commands.add_parser('publish')
    publisher.add_argument('artifact', type=Path)
    publisher.add_argument('target', type=Path)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    args = parser.parse_args()
    if args.command == 'run':
        if not args.scons or min(args.jobs, args.blocks, args.aa_blocks, args.build_timeout, args.runtime_timeout) < 1 or not math.isfinite(args.seconds) or args.seconds <= 0:
            parser.error('SCons and positive budgets are required')
        args.scons = str(Path(args.scons).resolve(strict=True))
        run(args)
    else:
        publish(args)


if __name__ == '__main__':
    main()

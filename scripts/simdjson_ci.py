#!/usr/bin/env python3
"""Measure pinned simdjson scalar/SSE library compilation and DOM parsing."""
import argparse
import json
import math
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess

import benchmark_timing as timing
import index_cpp_benchmarks as measure
import index_cpp_ci as common
import public_benchmarks

ROOT = Path(__file__).resolve().parents[1]
COMMIT = '40a686ab89dc5ee58ec10d0514dd5c99abbc472b'  # v5.1.1
UPSTREAM = 'https://github.com/simdjson/simdjson.git'
LABELS = ('cppgm', 'gcc', 'clang')
INPUTS = ('twitter.json', 'citm_catalog.json')
MODES = ('scalar', 'sse')


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def source_check(source):
    revision = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    changes = subprocess.check_output(['git', '-C', str(source), 'status', '--porcelain'], text=True)
    if revision != COMMIT or changes:
        raise ValueError('simdjson must be the clean pinned upstream checkout')


def fetch(source):
    source.mkdir(parents=True)
    for command in [('init', '-q'), ('remote', 'add', 'origin', UPSTREAM),
                    ('fetch', '-q', '--depth', '1', 'origin', COMMIT),
                    ('checkout', '-q', '--detach', 'FETCH_HEAD')]:
        subprocess.run(['git', '-C', str(source), *command], check=True)


def flags(source, mode):
    backend = 'westmere' if mode == 'sse' else 'fallback'
    return ['-std=c++17', '-O3', '-g0', '-DNDEBUG', '-I' + str(source / 'include'), '-I' + str(source / 'src'),
            '-DSIMDJSON_IMPLEMENTATION_HASWELL=0', '-DSIMDJSON_IMPLEMENTATION_ICELAKE=0',
            '-DSIMDJSON_IMPLEMENTATION_WESTMERE=' + str(int(mode == 'sse')),
            '-DSIMDJSON_IMPLEMENTATION_FALLBACK=' + str(int(mode == 'scalar')),
            '-DSIMDJSON_BUILTIN_IMPLEMENTATION=' + backend] + (['-mpopcnt'] if mode == 'sse' else [])


def headers(source, compilers):
    import shlex
    result = {}
    for mode in MODES:
        for label in ('gcc', 'clang'):
            text = subprocess.check_output([str(compilers[label]), *flags(source, mode), '-M',
                                            str(source / 'src/simdjson.cpp')], text=True)
            paths = shlex.split(text.split(':', 1)[1].replace('\\\n', ' '))
            result[mode + '-' + label] = {str(Path(p).resolve()): timing.sha(p) for p in sorted(set(paths))}
    return result


def summarize(rows, key):
    groups = []
    for block in sorted({r['block'] for r in rows}):
        group = [r for r in rows if r['block'] == block]
        a = statistics.mean(r[key] for r in group if r['side'] == 'A')
        b = statistics.mean(r[key] for r in group if r['side'] == 'B')
        if min(a, b) <= 0 or not all(math.isfinite(x) for x in (a, b)):
            raise ValueError('invalid timing interval')
        groups.append(a / b)
    return dict(speed=statistics.median(groups), blocks=groups)


def paired(one, reference, blocks, aa_blocks, keys):
    before = timing.observations(one, reference, reference, aa_blocks, phase='AA-before-' + reference)
    rows = timing.observations(one, reference, 'cppgm', blocks, phase='AB-' + reference)
    after = timing.observations(one, reference, reference, aa_blocks, phase='AA-after-' + reference)
    return {key: dict(summarize(rows, key), calibration_before=summarize(before, key),
                      calibration_after=summarize(after, key)) for key in keys}


def compile_mode(args, directory, source, compilers, mode, cpus):
    objects = {label: directory / (mode + '-' + label + '.o') for label in LABELS}
    digests = {}
    serial = 0

    def one(label, phase, block, position):
        nonlocal serial
        cmd = [str(compilers[label]), *flags(source, mode), '-c', str(source / 'src/simdjson.cpp'),
               '-o', str(objects[label])]
        row = measure.run(cmd, directory / f'compile-{mode}-{serial:03}-{label}', cpus, 900, counters=False)
        serial += 1
        row.update(mode=mode, label=label, phase=phase, block=block, position=position)
        if row['status'] == 0:
            row['object_sha256'] = timing.sha(objects[label])
        timing.retain(directory / 'compile-observations.jsonl', row)
        if row['status'] or row.get('timed_out'):
            raise ValueError('library compilation failed: ' + mode + '-' + label)
        digest = row['object_sha256']
        if digests.setdefault(label, digest) != digest:
            raise ValueError('repeated compilation changed its output')
        return row

    for label in LABELS:
        one(label, 'warmup', 0, 0)
    results = {ref: paired(one, ref, args.blocks, args.aa_blocks, ('cpu', 'wall')) for ref in ('gcc', 'clang')}
    driver = directory / ('driver-' + mode + '.o')
    with (directory / ('driver-' + mode + '.log')).open('w') as log:
        subprocess.run([str(compilers['gcc']), *flags(source, mode), '-c',
                        str(ROOT / 'benchmarks/simdjson/parse.cpp'), '-o', str(driver)],
                       stdout=log, stderr=subprocess.STDOUT, check=True)
    executables = {}
    for label in LABELS:
        exe = directory / ('parse-' + mode + '-' + label)
        with (directory / ('link-' + mode + '-' + label + '.log')).open('w') as log:
            subprocess.run([str(compilers['gcc']), str(driver), str(objects[label]), '-pthread', '-o', str(exe)],
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        executables[label] = exe
    return results, executables, digests


def runtime_mode(args, directory, source, executables, mode, cpus):
    results, checks = {}, {}
    serial = 0
    for name in INPUTS:
        input_path = source / 'jsonexamples' / name
        checks[name] = {}
        for label, exe in executables.items():
            output = directory / ('check-' + mode + '-' + label + '-' + name)
            with output.open('wb') as stream:
                subprocess.run(['taskset', '-c', cpus, str(exe), str(input_path), 'check', '0'],
                               stdout=stream, check=True, timeout=60)
            checks[name][label] = timing.sha(output)
        if len(set(checks[name].values())) != 1:
            raise ValueError('canonical DOM output differs: ' + mode + '-' + name)
        iterations = 32

        def sample(label):
            command = ['taskset', '-c', cpus, str(executables[label]), str(input_path), 'time', str(iterations)]
            run = subprocess.run(command, capture_output=True, text=True, timeout=180, check=True)
            row = json.loads(run.stdout)
            if row['iterations'] != iterations or row['bytes'] != input_path.stat().st_size:
                raise ValueError('driver did not execute the requested workload')
            return row

        # Calibrate outside the paired blocks, then freeze one repeat count for
        # all compilers. The timed loop excludes loading, allocation and rendering.
        while True:
            probe = sample('gcc')
            timing.retain(directory / 'runtime-observations.jsonl', dict(probe, mode=mode, input=name,
                                                                       label='gcc', phase='repeat-calibration'))
            if probe['cpu'] >= args.seconds:
                break
            iterations *= 2
            if iterations > 1048576:
                raise ValueError('could not obtain a meaningful runtime interval')

        def one(label, phase, block, position):
            nonlocal serial
            row = sample(label)
            row.update(mode=mode, input=name, label=label, phase=phase, block=block, position=position)
            serial += 1
            timing.retain(directory / 'runtime-observations.jsonl', row)
            return row

        for label in LABELS:
            one(label, 'warmup', 0, 0)
        results[name] = {ref: paired(one, ref, args.blocks, args.aa_blocks, ('cpu', 'wall'))
                         for ref in ('gcc', 'clang')}
    return results, checks


def validate(document):
    if document.get('status') != 'complete' or document.get('source_commit') != COMMIT:
        raise ValueError('incomplete or unpinned simdjson results')
    if set(document['modes']) != set(MODES):
        raise ValueError('missing backend results')
    for mode in MODES:
        data = document['modes'][mode]
        if set(data['runtime']) != set(INPUTS) or set(data['correctness']) != set(INPUTS):
            raise ValueError('missing runtime workloads')
        for hashes in data['correctness'].values():
            if set(hashes) != set(LABELS) or len(set(hashes.values())) != 1:
                raise ValueError('canonical output mismatch')
        if set(data['compilation']) != {'gcc', 'clang'}:
            raise ValueError('missing compilation comparison')
        for ref in ('gcc', 'clang'):
            for values in [data['compilation'][ref], *(data['runtime'][name][ref] for name in INPUTS)]:
                for key in ('cpu', 'wall'):
                    if not values[key]['blocks'] or not math.isfinite(values[key]['speed']) or values[key]['speed'] <= 0:
                        raise ValueError('invalid performance ratio')

    for name in INPUTS:
        if len({value for mode in MODES for value in document['modes'][mode]['correctness'][name].values()}) != 1:
            raise ValueError('scalar/SSE canonical output mismatch')


def report(document):
    text = '# simdjson benchmarks\n\n'
    if document['status'] != 'complete':
        return text + '**Incomplete:** ' + document.get('error', 'running') + '\n'
    validate(document)
    metadata = document['metadata']
    text += (f"Compiler: `{metadata['revision']}` · CPU: {metadata['cpu']}\n\n"
             f"Unmodified simdjson v5.1.1 (`{COMMIT}`), C++17 `-O3 -g0 -DNDEBUG`. "
             'Each backend is compiled separately; the SSE profile enables `-mpopcnt` for all three compilers. '
             'Compile times cover `src/simdjson.cpp` to an object; '
             'runtime covers repeated DOM parsing of hot input with a reused parser. '
             'One GCC-built driver per backend isolates library code generation. '
             'All three libraries produce identical canonical JSON for both inputs.\n\n'
             'Ratios are reference time / cppgm time: **higher is better**. '
             'Medians of paired ABBA/BAAB block ratios; A/A calibration before and after each comparison.\n\n'
             '| Backend | Measurement | vs GCC CPU | vs Clang CPU | vs GCC wall | vs Clang wall |\n'
             '|---|---|---:|---:|---:|---:|\n')
    warnings = []
    for mode in MODES:
        data = document['modes'][mode]
        for name, values in [('Compile', data['compilation']), *[(n, data['runtime'][n]) for n in INPUTS]]:
            ratios = [f"{values[ref][key]['speed']:.3f}×" for key in ('cpu', 'wall') for ref in ('gcc', 'clang')]
            text += f"| {mode} | {name} | " + ' | '.join(ratios) + ' |\n'
            for ref in ('gcc', 'clang'):
                for key in ('cpu', 'wall'):
                    for phase in ('before', 'after'):
                        ratio = values[ref][key]['calibration_' + phase]['speed']
                        if abs(ratio - 1) > .02:
                            warnings.append(f'{mode} {name} {ref} {key} A/A {phase}: {ratio:.3f}×')
    text += '\nCalibration variation above 2%: ' + ('; '.join(warnings) if warnings else 'none') + '. No observations are dropped.\n'
    text += '\n## Toolchains\n\n' + '\n'.join(f"- {label}: {version.splitlines()[0]}"
                                                    for label, version in metadata['toolchains'].items()) + '\n'
    return text + ('\n[Results and per-block ratios](results.json) · [Provenance and header hashes](metadata.json) · '
                   '[Compile observations](compile-observations.jsonl) · [Runtime observations](runtime-observations.jsonl)\n')


def run(args):
    directory = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=False)
    document = dict(status='running', source_commit=COMMIT)
    source = args.source.resolve() if args.source else directory / 'source'
    save(directory / 'results.json', document)
    try:
        if args.source is None:
            fetch(source)
        source_check(source)
        compiler = directory / 'compiler'
        shutil.copy2(args.compiler.resolve(strict=True), compiler)
        compiler.chmod(0o555)
        compilers = dict(cppgm=compiler, gcc=common.compiler_path(args.gcc), clang=common.compiler_path(args.clang))
        cpus = args.cpus or str(min(os.sched_getaffinity(0)))
        hashes = {label: timing.sha(path) for label, path in compilers.items()}
        metadata = dict(revision=subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
                        dirty=bool(subprocess.check_output(['git', '-C', str(ROOT), 'status', '--porcelain'])),
                        cpu=public_benchmarks.cpu_model(), cpus=cpus, flags={m: flags(source, m) for m in MODES},
                        compiler_hashes=hashes, driver_sha256=timing.sha(ROOT / 'benchmarks/simdjson/parse.cpp'),
                        input_hashes={n: timing.sha(source / 'jsonexamples' / n) for n in INPUTS},
                        toolchains={l: subprocess.check_output([str(p), '--version'], text=True).strip()
                                    for l, p in compilers.items()}, headers=headers(source, compilers))
        save(directory / 'metadata.json', metadata)
        document.update(metadata=metadata, modes={})
        for mode in MODES:
            compilation, executables, objects = compile_mode(args, directory, source, compilers, mode, cpus)
            runtime, correctness = runtime_mode(args, directory, source, executables, mode, cpus)
            document['modes'][mode] = dict(compilation=compilation, runtime=runtime, correctness=correctness,
                                           object_hashes=objects)
            save(directory / 'results.json', document)
        source_check(source)
        if hashes != {label: timing.sha(path) for label, path in compilers.items()} or headers(source, compilers) != metadata['headers']:
            raise ValueError('compiler/source/header epoch changed')
        document['status'] = 'complete'
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
    if 'AMD EPYC' not in metadata['cpu']:
        raise ValueError('EPYC publication requires EPYC hardware')
    files = ('results.json', 'metadata.json', 'compile-observations.jsonl', 'runtime-observations.jsonl')
    payloads = {name: (args.artifact / name).read_bytes() for name in files}
    details = report(document) + '\n[CI run and complete artifacts](' + args.run_url + ')\n'
    for dest in (args.target / 'simdjson/epyc', args.target / 'simdjson/history' / args.revision):
        dest.mkdir(parents=True, exist_ok=True)
        for name, content in payloads.items():
            (dest / name).write_bytes(content)
        (dest / 'report.md').write_text(details)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--compiler', type=Path, default=ROOT / 'dev/cppgm++')
    runner.add_argument('--source', type=Path)
    runner.add_argument('--output', type=Path, required=True)
    runner.add_argument('--gcc', default='g++')
    runner.add_argument('--clang', default='clang++')
    runner.add_argument('--cpus')
    runner.add_argument('--blocks', type=int, default=3)
    runner.add_argument('--aa-blocks', type=int, default=1)
    runner.add_argument('--seconds', type=float, default=.35)
    publisher = commands.add_parser('publish')
    publisher.add_argument('artifact', type=Path)
    publisher.add_argument('target', type=Path)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    args = parser.parse_args()
    if args.command == 'run':
        if args.blocks < 1 or args.aa_blocks < 1 or args.seconds <= 0 or not math.isfinite(args.seconds):
            parser.error('positive block counts and measurement interval required')
        run(args)
    else:
        publish(args)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Measure fixed index-cpp executables using the original checksummed inputs."""

import argparse
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import statistics
import subprocess
import time

import benchmark_timing as timing
from run_ab_compile_benchmark import load_snapshot


UPSTREAM_COMMIT = '79e0b423094d420da37fb714f20c53fc35eeb2a3'
SOURCE_SHA256 = '8845faa63711d9065041f05f483e93cae1dc9fc36b02d7a4e15376bdbfae4c89'
COUNTERS = ('instructions:u', 'cycles:u', 'branches:u', 'branch-misses:u')
CASE = re.compile(r'^(.+): (OK|ERR\[.*\]) in ([\d.]+)s$', re.M)
SUMMARY = re.compile(r'^Summary: ([\d.]+)s, (\d+), (\d+), (\d+)$', re.M)


def parse_output(text, expected=None):
    cases = {}
    for name, status, seconds in CASE.findall(text):
        if status != 'OK' or name in cases:
            raise ValueError('failed or duplicate workload: ' + name)
        cases[name] = float(seconds)
    summary = SUMMARY.search(text)
    if (summary is None or not cases or
            tuple(map(int, summary.groups()[1:])) != (len(cases), len(cases), 0)):
        raise ValueError('missing or inconsistent checksum results')
    if expected is not None and set(cases) != set(expected):
        raise ValueError('workload inventory changed')
    return cases


def parse_counters(path):
    counts = {}
    for line in path.read_text().splitlines():
        fields = line.split(';')
        if len(fields) < 5 or fields[2] not in COUNTERS:
            continue
        if (not fields[0].isdigit() or not math.isfinite(float(fields[4])) or
                float(fields[4]) < 99.5):
            raise ValueError('unavailable or multiplexed counter: ' + line)
        counts[fields[2].removesuffix(':u')] = int(fields[0])
    if len(counts) != len(COUNTERS):
        raise ValueError('incomplete performance counters')
    return counts


def run(command, prefix, cpus, timeout, *, test=False, counters=True):
    env = dict(os.environ)
    env['LC_ALL'] = 'C'
    env.pop('TEST', None)
    if test:
        env['TEST'] = '1'
    prefix = str(prefix)
    invocation = ['taskset', '-c', cpus, '/usr/bin/time', '-f', '%e;%U;%S;%M',
                  '-o', prefix + '.time']
    if counters:
        invocation += ['perf', 'stat', '-x', ';', '-o', prefix + '.perf',
                       '-e', ','.join(COUNTERS), '--']
    invocation += list(map(str, command))
    before = load_snapshot()
    started = time.perf_counter()
    with open(prefix + '.stdout', 'w') as output, open(prefix + '.stderr', 'w') as error:
        process = subprocess.Popen(invocation, stdout=output, stderr=error,
                                   env=env, start_new_session=True)
        try:
            status = process.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            status = 'timeout'
    row = dict(command=invocation, status=status, wall=time.perf_counter() - started,
               load_before=before, load_after=load_snapshot(),
               stdout=prefix + '.stdout', stderr=prefix + '.stderr')
    if status == 0:
        wall, user, system, rss = Path(prefix + '.time').read_text().strip().split(';')
        row.update(cpu=float(user) + float(system), timed_wall=float(wall), max_rss_kib=int(rss))
        if counters:
            try:
                row.update(parse_counters(Path(prefix + '.perf')))
            except ValueError as error:
                row['counter_error'] = str(error)
    return row


def paired_speed(rows, value):
    ratios = []
    for block in sorted({row['block'] for row in rows}):
        group = [row for row in rows if row['block'] == block]
        a = statistics.mean(value(row) for row in group if row['side'] == 'A')
        b = statistics.mean(value(row) for row in group if row['side'] == 'B')
        ratios.append(a / b if b > 0 else None)
    return dict(speed=statistics.median(ratios) if all(r is not None for r in ratios) else None,
                blocks=ratios)


def summarize(rows, workloads):
    loops = {name: paired_speed(rows, lambda row: row['cases'][name]) for name in workloads}
    speeds = [result['speed'] for result in loops.values()]
    result = dict(loops=loops, process_cpu=paired_speed(rows, lambda row: row['cpu']),
                  geometric_speed=math.exp(statistics.mean(math.log(s) for s in speeds))
                  if all(s is not None and s > 0 for s in speeds) else None)
    if all('instructions' in row for row in rows):
        result['process_instructions'] = paired_speed(rows, lambda row: row['instructions'])
    return result


def drift_warnings(calibration, tolerance):
    return [name for name, result in calibration['loops'].items()
            if result['speed'] is None or abs(result['speed'] - 1) > tolerance]


def measure(args):
    checkout = args.source_root.resolve(strict=True)
    commit = subprocess.check_output(['git', '-C', str(checkout), 'rev-parse', 'HEAD'], text=True).strip()
    if commit != UPSTREAM_COMMIT or timing.sha(checkout / 'index.cpp') != SOURCE_SHA256:
        raise ValueError('index-cpp source differs from the pinned upstream')
    binaries = {}
    for spelling in args.binary:
        label, path = spelling.split('=', 1)
        if not re.fullmatch(r'[A-Za-z0-9_-]+', label) or label in binaries:
            raise ValueError('invalid or duplicate executable label: ' + label)
        binaries[label] = Path(path).resolve(strict=True)
    references = args.reference or [label for label in binaries if label != args.candidate]
    if args.candidate not in binaries or not references or any(label not in binaries for label in references):
        raise ValueError('candidate and references must name supplied executables')
    directory = args.out.resolve()
    directory.mkdir(parents=True, exist_ok=False)
    snapshots = directory / 'bin'
    snapshots.mkdir()
    for label, source in binaries.items():
        target = snapshots / label
        shutil.copy2(source, target)
        target.chmod(0o555)
        binaries[label] = target
    identities = {label: timing.sha(path) for label, path in binaries.items()}
    report = dict(status='running', source_commit=commit, source_sha256=SOURCE_SHA256,
                  executables={label: dict(path=str(path), sha256=identities[label])
                               for label, path in binaries.items()},
                  cpus=args.cpus, test_input=False, blocks=args.blocks, pairs={}, calibration={},
                  drift_warning=args.drift_warning, timing_warnings={})
    if args.metadata:
        report['build_metadata'] = json.loads(args.metadata.read_text())
    serial = 0

    def save():
        (directory / 'report.json').write_text(json.dumps(report, indent=2) + '\n')

    def check_identities():
        if identities != {label: timing.sha(path) for label, path in binaries.items()}:
            raise ValueError('a measured executable changed')
        if timing.sha(checkout / 'index.cpp') != SOURCE_SHA256:
            raise ValueError('upstream source changed')

    try:
        save()
        universe = None
        for label, program in binaries.items():
            row = run([program], directory / ('check-' + label), args.cpus, args.timeout,
                      test=True, counters=False)
            timing.retain(directory / 'observations.jsonl', dict(row, phase='correctness', label=label))
            if row['status'] != 0:
                raise ValueError('checksum run failed: ' + label)
            cases = parse_output(Path(row['stdout']).read_text(), universe)
            if len(cases) != 50:
                raise ValueError('expected all 50 upstream checksum workloads')
            universe = set(cases)
        filters = args.only or ['']
        selected = [sorted(name for name in universe if pattern.lower() in name.lower()) for pattern in filters]
        if any(not group for group in selected):
            raise ValueError('an upstream workload filter matched no cases')
        if sum(map(len, selected)) != len(set().union(*map(set, selected))):
            raise ValueError('overlapping workload filters would repeat observations')
        report['workloads'] = sorted(set().union(*map(set, selected)))

        for reference in references:
            for pattern, names in zip(filters, selected):
                key = reference + ':' + (pattern or 'all')

                def one(label, phase, block, position):
                    nonlocal serial
                    check_identities()
                    prefix = directory / f'run-{serial:04}-{label}'
                    serial += 1
                    row = run([binaries[label], *([pattern] if pattern else [])], prefix,
                              args.cpus, args.timeout, counters=not args.no_counters)
                    row.update(label=label, phase=phase, block=block, position=position, pair=key)
                    try:
                        if row['status'] != 0 or row.get('counter_error'):
                            raise ValueError('workload process failed: ' + str(prefix))
                        row['cases'] = parse_output(Path(row['stdout']).read_text(), names)
                    finally:
                        timing.retain(directory / 'observations.jsonl', row)
                    return row

                aa = timing.observations(one, reference, reference, args.aa_blocks, phase='AA')
                report['calibration'][key] = summarize(aa, names)
                warnings = drift_warnings(report['calibration'][key], args.drift_warning)
                if warnings:
                    report['timing_warnings'][key] = warnings
                    print('A/A drift warning:', key, ', '.join(warnings), flush=True)
                save()
                paired = timing.observations(one, reference, args.candidate, args.blocks)
                report['pairs'][key] = summarize(paired, names)
                save()
                print(key, json.dumps(report['pairs'][key]), flush=True)
        check_identities()
        report['status'] = 'complete'
    except BaseException as error:
        report.update(status='failed', error=repr(error))
        raise
    finally:
        save()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True, help='new receipt directory')
    parser.add_argument('--binary', action='append', required=True, metavar='LABEL=PATH')
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--reference', action='append')
    parser.add_argument('--only', action='append', help='upstream substring filter; default all 50')
    parser.add_argument('--cpus', required=True, help='taskset CPU list; allow enough cores for threaded cases')
    parser.add_argument('--blocks', type=int, default=3)
    parser.add_argument('--aa-blocks', type=int, default=1)
    parser.add_argument('--timeout', type=float, default=600)
    parser.add_argument('--no-counters', action='store_true')
    parser.add_argument('--drift-warning', type=float, default=.02, help='A/A loop-time warning tolerance')
    parser.add_argument('--metadata', type=Path, help='build commands/toolchains/header manifests JSON')
    args = parser.parse_args()
    if args.blocks < 1 or args.aa_blocks < 1 or args.timeout <= 0 or args.drift_warning < 0:
        parser.error('block counts and timeout must be positive')
    measure(args)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Check original optimization kernels against one host-built scalar driver."""
import argparse
import hashlib
import json
import os
import platform
from pathlib import Path
import shlex
import shutil
import signal
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
EDGES = (0, 1, 3, 4, 15, 16, 17, 18, 63, 64, 65, 257)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def settings(path):
    data = dict(line.split('=', 1) for line in path.read_text().splitlines()
                if line and not line.startswith('#'))
    if set(data) != {'case', 'level', 'iterations', 'scenarios', 'lowir_limit', 'native_limit'}:
        raise ValueError(f'{path}: incomplete or unknown quality settings')
    for key in ('case', 'level', 'iterations'):
        data[key] = int(data[key])
    if data['level'] not in (1, 2, 3) or data['iterations'] < 1 or not 1 <= data['case'] <= 12:
        raise ValueError(f'{path}: invalid case, level or iteration count')
    data['scenarios'] = [int(n) for n in data['scenarios'].split(',')]
    if not data['scenarios'] or any(n < 0 or n > 275 for n in data['scenarios']):
        raise ValueError(f'{path}: invalid scenarios')
    for key in ('lowir_limit', 'native_limit'):
        data[key] = float(data[key])
        if not 0 < data[key] < float('inf'):
            raise ValueError(f'{path}: invalid limit')
    return data


def work_ratio(candidate, reference):
    candidate_work = candidate[1] - candidate[0]
    reference_work = reference[1] - reference[0]
    if candidate_work <= 0 or reference_work <= 0:
        raise ValueError('startup-subtracted instruction counts must be positive')
    return candidate_work / reference_work


class Run:
    def __init__(self, directory):
        self.directory = directory
        self.observations = []

    def command(self, command, *, timeout=60, phase='build', annotation=None):
        start = time.monotonic()
        number = len(self.observations)
        rss = self.directory / f'{number:04}.rss'
        measured = ['/usr/bin/time', '-f', '%M', '-o', str(rss), *map(str, command)] if phase == 'build' and platform.system() == 'Linux' and Path('/usr/bin/time').is_file() else list(map(str, command))
        proc = subprocess.Popen(measured, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, start_new_session=True)
        try:
            stdout, stderr = proc.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid, signal.SIGKILL)
            stdout, stderr = proc.communicate()
            stderr += b'\ncommand timed out\n'
        (self.directory / f'{number:04}.stdout').write_bytes(stdout)
        (self.directory / f'{number:04}.stderr').write_bytes(stderr)
        row = dict(command=list(map(str, command)), phase=phase,
                   wall=time.monotonic() - start, status=proc.returncode,
                   stdout_sha256=hashlib.sha256(stdout).hexdigest())
        if rss.is_file() and rss.read_text().strip().isdigit():
            row['peak_rss_kib'] = int(rss.read_text())
        row.update(annotation or {})
        self.observations.append(row)
        with (self.directory / 'observations.jsonl').open('a') as stream:
            stream.write(json.dumps(row) + '\n')
        if proc.returncode:
            raise RuntimeError(f'{phase}: command failed ({proc.returncode}): {command}\n'
                               + stderr.decode(errors='replace')[-2000:])
        return stdout

    def count(self, executable, iterations, scenario):
        output = self.directory / f'cachegrind-{len(self.observations)}.out'
        command = ['valgrind', '--tool=cachegrind', '--cache-sim=no',
                   f'--cachegrind-out-file={output}', executable, 'actual', iterations, scenario]
        result = self.command(command, timeout=180, phase='count')
        rows = output.read_text().splitlines()
        events = next(row.split()[1:] for row in rows if row.startswith('events:'))
        counts = next(row.split()[1:] for row in rows if row.startswith('summary:'))
        return int(counts[events.index('Ir')]), result


def executable(command):
    words = shlex.split(command)
    found = shutil.which(words[0]) if words else None
    if not found:
        raise ValueError(f'executable not found: {command}')
    # Reference wrappers dispatch by argv[0]; resolving their symlink would
    # invoke run_reference_binary.sh under the wrong name in a student export.
    return [str(Path(found).absolute()), *words[1:]]


def check_fixture(args, fixture, output, tools):
    config = settings(fixture.with_suffix('.quality'))
    level = args.level or config['level']
    directory = output / fixture.stem
    directory.mkdir()
    run = Run(directory)
    # Freeze sources together, preserving the include path used by each lane.
    inputs = directory / 'inputs'
    (inputs / 'kernels').mkdir(parents=True)
    (inputs / 'improvements').mkdir()
    shutil.copy2(ROOT / 'pa33/tests/kernels/600-support.h', inputs / 'kernels/600-support.h')
    source = inputs / fixture.parent.name / fixture.name
    shutil.copy2(fixture, source)
    data = dict(settings=config, level=level, sources={str(p.relative_to(inputs)): digest(p)
                for p in inputs.rglob('*') if p.is_file()}, checks=0)
    driver = directory / 'driver.o'
    flags = ['-std=c++11', f'-O{level}', '-g0', '-fno-lto']
    run.command([*tools['host'], '-std=c++11', '-O2', '-DQUALITY_DRIVER',
                 f'-DQUALITY_CASE={config["case"]}', '-x', 'c++', '-c',
                 inputs / 'kernels/600-support.h', '-o', driver])
    reference = directory / 'gcc.o'
    run.command([*tools['host'], *flags, '-x', 'c++', '-c', source, '-o', reference])
    candidate = directory / 'candidate.o'
    if args.mode == 'lowir':
        raw, optimized = directory / 'raw.lowir', directory / 'optimized.lowir'
        run.command([*tools['frontend'], '--emit-lowir', '-O0', '-g0', '-o', raw, source])
        run.command([*tools['optimizer'], f'-O{level}', '-o', optimized, raw])
        run.command([*tools['backend'], '-c', '-O0', '-g0', '-o', candidate, optimized])
    else:
        run.command([*tools['compiler'], *flags, '-x', 'c++', '-c', source, '-o', candidate])
    binaries = {}
    for name, obj in [('gcc', reference), ('candidate', candidate)]:
        path = directory / name
        run.command([*tools['host'], driver, obj, '-lm', '-o', path])
        binaries[name] = path
    data['executables'] = {name: digest(path) for name, path in binaries.items()}
    iterations = args.iterations or max(257, config['iterations'] // args.scale)
    data['iterations'] = iterations
    for scenario in config['scenarios']:
        for n in sorted(set((*EDGES, iterations))):
            expected = run.command([binaries['gcc'], 'oracle', n, scenario], phase='oracle')
            for name, path in binaries.items():
                actual = run.command([path, 'actual', n, scenario], phase='behavior')
                if expected != actual:
                    raise RuntimeError(f'{fixture}: wrong result at {n} iterations, scenario {scenario}: '
                                       f'{name}: {actual!r}, expected {expected!r}')
                data['checks'] += 1
    # Core fixtures measure each alias scenario. Optional exercises report one
    # longest disjoint workload after checking correctness for every scenario.
    representative = max(config['scenarios'], key=lambda s: (s >> 2, -(s & 3)))
    scenarios = [representative] if args.report_only else config['scenarios']
    measurements = []
    for scenario in scenarios:
        counts = {}
        for name, path in binaries.items():
            counts[name] = []
            for n in (0, iterations):
                count, actual = run.count(path, n, scenario)
                expected = run.command([binaries['gcc'], 'oracle', n, scenario], phase='oracle')
                if actual != expected:
                    raise RuntimeError(f'{fixture}: wrong result under Cachegrind')
                counts[name].append(count)
        measurements.append(dict(scenario=scenario, counts=counts,
                                 ratio=work_ratio(counts['candidate'], counts['gcc'])))
    data['measurements'] = measurements
    data['ratio'] = max(m['ratio'] for m in measurements)
    data['limit'] = config[f'{args.mode}_limit']
    data['status'] = 'PASS' if args.report_only or data['ratio'] <= data['limit'] else 'FAIL'
    if args.timing_rounds:
        # A/A first, then paired ABBA blocks. Nothing is built while timing.
        for phase, rounds in [('AA', 2), ('AB', args.timing_rounds)]:
            for block in range(rounds):
                for position, side in enumerate('ABBA' if block % 2 == 0 else 'BAAB'):
                    name = 'gcc' if phase == 'AA' or side == 'A' else 'candidate'
                    actual = run.command([binaries[name], 'actual', iterations, scenario], phase='timing',
                                         annotation=dict(pair=phase, block=block, side=side, position=position))
                    if actual != expected:
                        raise RuntimeError('timed executable changed its output')
        (directory / 'observations.json').write_text(json.dumps(run.observations, indent=2) + '\n')
    for name, path in binaries.items():
        if digest(path) != data['executables'][name]:
            raise RuntimeError('an executable changed during measurement')
    (directory / 'observations.json').write_text(json.dumps(run.observations, indent=2) + '\n')
    (directory / 'result.json').write_text(json.dumps(data, indent=2) + '\n')
    return data


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('tests', type=Path)
    parser.add_argument('--mode', choices=('lowir', 'native'), default='lowir')
    parser.add_argument('--frontend', default=str(ROOT / 'dev/cppgm++-ref'))
    parser.add_argument('--optimizer', default=str(ROOT / 'dev/lowiropt'))
    parser.add_argument('--backend', default=str(ROOT / 'dev/cppgm++-ref'))
    parser.add_argument('--compiler', default=str(ROOT / 'dev/cppgm++'))
    parser.add_argument('--host', default='g++', help='GCC driver for the common oracle and comparison')
    parser.add_argument('--out', type=Path, default=ROOT / 'pa33/obj/quality')
    parser.add_argument('--scale', type=int, default=1)
    parser.add_argument('--iterations', type=int)
    parser.add_argument('--level', type=int, choices=(1, 2, 3))
    parser.add_argument('--timing-rounds', type=int, default=0)
    parser.add_argument('--report-only', action='store_true')
    parser.add_argument('--list', action='store_true')
    args = parser.parse_args(argv)
    if args.scale < 1 or args.timing_rounds < 0 or (args.iterations is not None and args.iterations < 1):
        parser.error('scale must be positive and timing rounds nonnegative')
    fixtures = sorted(args.tests.rglob('*.t')) if args.tests.is_dir() else [args.tests]
    if not fixtures or any(not p.is_file() for p in fixtures):
        parser.error('no quality fixtures found')
    if args.list:
        print('\n'.join(map(str, fixtures)))
        return 0
    names = ('host', 'frontend', 'optimizer', 'backend') if args.mode == 'lowir' else ('host', 'compiler')
    tools = {name: executable(getattr(args, name)) for name in names}
    args.out.mkdir(parents=True, exist_ok=True)
    # Each invocation retains its observations, rather than overwriting a run.
    output = args.out / f'{args.mode}-{time.time_ns()}'
    output.mkdir()
    tool_hashes = {name: digest(command[0]) for name, command in tools.items()}
    metadata = dict(tools=tools, tool_sha256=tool_hashes, platform=platform.platform(),
                    host_version=subprocess.check_output([*tools['host'], '--version'], text=True),
                    mode=args.mode, report_only=args.report_only, scale=args.scale)
    cpuinfo = Path('/proc/cpuinfo')
    metadata['cpu'] = next((line.split(':', 1)[1].strip()
                            for line in cpuinfo.read_text().splitlines()
                            if line.startswith('model name')), platform.processor()) if cpuinfo.exists() else platform.processor()
    if hasattr(os, 'sched_getaffinity'):
        metadata['cpu_affinity'] = sorted(os.sched_getaffinity(0))
    (output / 'environment.json').write_text(json.dumps(metadata, indent=2) + '\n')
    results = []
    for fixture in fixtures:
        try:
            row = check_fixture(args, fixture.resolve(), output, tools)
        except (ValueError, RuntimeError, OSError, StopIteration) as error:
            row = dict(status='FAIL', error=str(error))
        row['fixture'] = str(fixture)
        results.append(row)
        (output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
        if row['status'] != 'PASS' or not os.environ.get('CPPGM_REPORT_QUIET'):
            print(f'{row["status"]} {fixture.name}: '
                  + (row.get('error') or f'{row["ratio"]:.4f}x GCC instructions ({args.mode})'), flush=True)
    if tool_hashes != {name: digest(command[0]) for name, command in tools.items()}:
        raise RuntimeError('a compiler or reference tool changed during the run')
    passed = sum(r['status'] == 'PASS' for r in results)
    if os.environ.get('KEEP_GOING') and os.environ.get('CPPGM_TEST_COUNTS_FILE'):
        with open(os.environ['CPPGM_TEST_COUNTS_FILE'], 'a') as counts:
            counts.write(f'{passed} {len(results)}\n')
    print(f'quality kernels: {passed}/{len(results)} pass; observations: {output}', flush=True)
    return int(passed != len(results))


if __name__ == '__main__':
    raise SystemExit(main())

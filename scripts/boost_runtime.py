"""Replay Boost native tests with verification outside paired CPU measurements."""
from concurrent.futures import ThreadPoolExecutor
import json
import math
import os
from pathlib import Path
import select
import signal
import statistics
import subprocess
import tempfile
import threading
import time

import benchmark_timing as timing

LABELS = ('cppgm', 'gcc', 'clang')
GDB = 'libs/json/test//_gdb-printers.test'


def eligible(row):
    return row['kind'] == 'RUN' and row['target'] != GDB


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def execute(command, cwd, directory, timeout):
    """wait4 attributes CPU to this child even when other workers finish."""
    stdout, stderr = directory / 'stdout', directory / 'stderr'
    started = time.monotonic()
    with stdout.open('wb') as out, stderr.open('wb') as err:
        child = subprocess.Popen(command, cwd=cwd, stdin=subprocess.DEVNULL,
                                 stdout=out, stderr=err, start_new_session=True)
        descriptor = os.pidfd_open(child.pid)
        try:
            poll = select.poll()
            poll.register(descriptor, select.POLLIN)
            timed_out = not poll.poll(math.ceil(timeout * 1000))
            if timed_out:
                os.killpg(child.pid, signal.SIGKILL)
            _, status, usage = os.wait4(child.pid, 0)
            child.returncode = os.waitstatus_to_exitcode(status)
        finally:
            os.close(descriptor)
    # File hashing is deliberately outside the process timer.
    elapsed = time.monotonic() - started
    return dict(cpu=usage.ru_utime + usage.ru_stime, wall=elapsed,
                code=child.returncode, timed_out=timed_out,
                stdout_sha256=timing.sha(stdout), stderr_sha256=timing.sha(stderr))


def totals(programs, project=None):
    return sum(row['cpu'] for row in programs
               if project is None or row['target'].split('//')[0] == project)


def comparison(observations, project=None):
    pairs = []
    for block in sorted({r['block'] for r in observations}):
        group = [r for r in observations if r['block'] == block]
        values = {side: statistics.mean(totals(r['programs'], project) for r in group if r['side'] == side)
                  for side in ('A', 'B')}
        if min(values.values()) <= 0:
            raise ValueError('missing positive paired Boost CPU time')
        pairs.append(dict(reference_seconds=values['A'], candidate_seconds=values['B'],
                          speedup=values['A'] / values['B']))
    speeds = [r['speedup'] for r in pairs]
    speedup = statistics.median(speeds)
    return dict(speedup=speedup, blocks=pairs,
                reference_seconds=statistics.median(r['reference_seconds'] for r in pairs),
                candidate_seconds=statistics.median(r['candidate_seconds'] for r in pairs),
                spread=max(speeds) / min(speeds) - 1)


def campaign(document, directory, jobs, blocks=2, timeout=180):
    directory.mkdir(exist_ok=False)
    selected, exclusions = {}, []
    for label in LABELS:
        rows = [r for s in document['compilers'][label]['suites'] for r in s['tests']]
        selected[label] = {r['target']: dict(target=r['target'], command=r['native_command'],
                                            cwd=r['native_cwd'], repeats=1)
                           for r in rows if eligible(r)}
        if label == 'cppgm':
            exclusions = [dict(target=r['target'], reason='debugger integration' if r['target'] == GDB
                               else 'expected-failure execution')
                          for r in rows if r['kind'] in ('RUN', 'RUN_FAIL') and not eligible(r)]
    targets = sorted(selected['cppgm'])
    if not targets or any(sorted(rows) != targets for rows in selected.values()):
        raise ValueError('Boost runtime requires identical nonempty native test sets')
    for rows in selected.values():
        for case in rows.values():
            case['executable_sha256'] = timing.sha(case['command'][0])
    manifest = dict(cases=selected, exclusions=exclusions, jobs=jobs, blocks=blocks,
                    affinity=sorted(os.sched_getaffinity(0)))
    save(directory / 'manifest.json', manifest)
    warmups, observations = [], []
    outputs = {target: set() for target in targets}
    sequence = 0

    def batch(label, phase, block, position):
        nonlocal sequence
        index = sequence
        sequence += 1
        rows = selected[label]
        # Immutable binary checks never enter the timed batch.
        for case in rows.values():
            if timing.sha(case['command'][0]) != case['executable_sha256']:
                raise ValueError('a Boost executable changed: ' + case['target'])
        receipt = directory / f'{index:03}-{phase}-{label}.jsonl'
        receipt.touch()
        receipt_lock = threading.Lock()

        def program(target):
            case = rows[target]
            samples = []
            with tempfile.TemporaryDirectory(dir=directory) as tmp:
                for repetition in range(case['repeats']):
                    row = execute(case['command'], case['cwd'], Path(tmp), timeout)
                    row.update(target=target, repetition=repetition)
                    # Keep every observation, including one that fails its oracle.
                    with receipt_lock:
                        timing.retain(receipt, row)
                    samples.append(row)
                    outputs[target].add((row['stdout_sha256'], row['stderr_sha256']))
                    if row['timed_out'] or row['code'] != 0:
                        failure = directory / f'failure-{index}-{targets.index(target)}'
                        Path(tmp).rename(failure)
                        raise ValueError('Boost runtime outcome changed: ' + target + '; ' + str(failure))
            return dict(target=target, cpu=statistics.mean(r['cpu'] for r in samples), samples=samples)

        started = time.monotonic()
        programs = list(pool.map(program, targets))
        wall = time.monotonic() - started
        # This batch wall includes orchestration/verification, so is diagnostic
        # only. The score uses wait4 child CPU, which excludes all of that work.
        for case in rows.values():
            if timing.sha(case['command'][0]) != case['executable_sha256']:
                raise ValueError('a Boost executable changed: ' + case['target'])
        row = dict(label=label, phase=phase, block=block, position=position,
                   cpu=totals(programs), batch_wall_diagnostic=wall, programs=programs,
                   observations=receipt.name)
        timing.retain(directory / 'batches.jsonl', {k: v for k, v in row.items() if k != 'programs'})
        print(f'Boost runtime {phase} {label}: {row["cpu"]:.3f}s CPU', flush=True)
        return row

    with ThreadPoolExecutor(max_workers=jobs) as pool:
        for iteration in range(2):
            for label in LABELS:
                warmups.append(batch(label, 'warmup', iteration, 0))
        for target in targets:
            samples = [p['samples'][0] for w in warmups for p in w['programs'] if p['target'] == target]
            # Native assertions are the upstream oracle. Unit tests can print
            # rounded times that happen to match in every warmup, then vary.
            # Retain output hashes without inventing an output oracle from them.
            duration = max(max(r['cpu'], r['wall']) for r in samples)
            repeats = max(1, min(10, math.ceil(.02 / max(duration, .000001))))
            for rows in selected.values():
                rows[target]['repeats'] = repeats
        save(directory / 'manifest.json', manifest)
        for reference in ('gcc', 'clang'):
            calibration = timing.observations(batch, reference, reference, 1, phase='AA-' + reference)
            paired = timing.observations(batch, reference, 'cppgm', blocks, phase='AB-' + reference)
            observations.append((reference, calibration, paired))

    suites = {t.split('//')[0] for t in targets}
    result = dict(status='passed', selected=len(targets), exclusions=exclusions,
                  blocks=blocks, jobs=jobs, output_equality_cases=sum(len(v) == 1 for v in outputs.values()),
                  case_set_sha256=timing.sha(directory / 'manifest.json'),
                  comparisons={}, suites={}, cases=[])
    for reference, calibration, paired in observations:
        result['comparisons'][reference] = dict(comparison(paired), calibration=comparison(calibration))
        for project in sorted(suites):
            suite = result['suites'].setdefault(project, dict(comparisons={}))
            suite['comparisons'][reference] = dict(comparison(paired, project),
                                                  calibration=comparison(calibration, project))
    measured = [r for _, _, paired in observations for r in paired]
    for target in targets:
        seconds = {label: statistics.median(p['cpu'] for r in measured if r['label'] == label
                                           for p in r['programs'] if p['target'] == target)
                   for label in LABELS}
        result['cases'].append(dict(target=target, repeats=selected['cppgm'][target]['repeats'], seconds=seconds,
                                     output_check='byte equality observed' if len(outputs[target]) == 1
                                     else 'native test oracle; output varies'))
    save(directory / 'summary.json', result)
    return result


def validate(result, rows):
    targets = {r['target'] for r in rows if eligible(r)}
    measured = result.get('cases', [])
    if (result.get('status') != 'passed' or result.get('selected') != len(targets)
            or len(measured) != len(targets) or {r['target'] for r in measured} != targets
            or result.get('blocks', 0) < 2):
        raise ValueError('incomplete Boost runtime cohort')
    excluded = {r['target'] for r in rows if r['kind'] in ('RUN', 'RUN_FAIL') and not eligible(r)}
    if {r['target'] for r in result['exclusions']} != excluded:
        raise ValueError('incorrect Boost runtime exclusions')
    groups = [result, *result['suites'].values()]
    if set(result['suites']) != {t.split('//')[0] for t in targets}:
        raise ValueError('missing Boost runtime suites')
    for group in groups:
        if set(group['comparisons']) != {'gcc', 'clang'}:
            raise ValueError('missing Boost runtime reference')
        for value in group['comparisons'].values():
            for summary, count in [(value, result['blocks']), (value['calibration'], 1)]:
                pairs = summary['blocks']
                if len(pairs) != count:
                    raise ValueError('missing Boost runtime paired block')
                for pair in pairs:
                    seconds = [pair[k] for k in ('candidate_seconds', 'reference_seconds')]
                    if any(type(x) not in (float, int) or not math.isfinite(x) or x <= 0 for x in seconds):
                        raise ValueError('invalid Boost runtime CPU measurement')
                    if not math.isclose(pair['speedup'], seconds[1] / seconds[0]):
                        raise ValueError('incorrect Boost runtime block ratio')
                if not math.isclose(summary['speedup'], statistics.median(p['speedup'] for p in pairs)):
                    raise ValueError('incorrect Boost runtime speedup')
    for row in measured:
        if type(row['repeats']) is not int or not 1 <= row['repeats'] <= 10 or set(row['seconds']) != set(LABELS):
            raise ValueError('invalid Boost runtime repetitions or compiler set')
        if any(not math.isfinite(x) or x < 0 for x in row['seconds'].values()):
            raise ValueError('invalid per-test Boost runtime CPU')


def noisy(summary):
    return (abs(summary['calibration']['speedup'] - 1) > .05
            or summary['spread'] > .05 or summary['calibration']['spread'] > .05)


def validate_evidence(result, directory):
    manifest = directory / 'manifest.json'
    batches = directory / 'batches.jsonl'
    summary = directory / 'summary.json'
    if not all(p.is_file() for p in (manifest, batches, summary)):
        raise ValueError('missing Boost runtime evidence')
    if timing.sha(manifest) != result['case_set_sha256'] or json.loads(summary.read_text()) != result:
        raise ValueError('Boost runtime evidence differs from its summary')
    rows = [json.loads(line) for line in batches.read_text().splitlines()]
    if len(rows) != 6 + 8 + 8 * result['blocks']:
        raise ValueError('missing Boost runtime observation')
    for row in rows:
        name = row['observations']
        if Path(name).name != name or not (directory / name).is_file():
            raise ValueError('missing Boost per-process runtime evidence')

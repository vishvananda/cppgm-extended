#!/usr/bin/env python3
"""Private controlled comparison; no result publication or main-branch changes."""
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import time

import benchmark_timing as timing
import simdjson_ci as suite

root = Path(__file__).resolve().parents[1]
out = root / 'obj/probe/results'
out.mkdir(parents=True)
source = root / 'obj/probe/upstream'
suite.fetch(source)
suite.source_check(source)
compilers = {label: root / ('obj/probe/cppgm-' + label)
             for label in ('arithmetic', 'candidate')}
compilers.update(gcc=Path('/usr/bin/g++'), clang=Path('/usr/bin/clang++-21'))
labels = tuple(compilers)
cpu = min(os.sched_getaffinity(0))
os.sched_setaffinity(0, {cpu})
metadata = dict(cpu_id=cpu, cpu=Path('/proc/cpuinfo').read_text(),
                revision=subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
                upstream=suite.COMMIT,
                compiler_sha256={k: timing.sha(v) for k, v in compilers.items()},
                versions={k: subprocess.check_output([str(v), '--version'], text=True) for k, v in compilers.items()},
                headers=suite.headers(source, compilers),
                revisions=dict(arithmetic='963dbc3c8f2c429179dd0e9686fdb1f2c1efebd0',
                               candidate='arithmetic plus bounded bit projection and vector mask range facts'))
suite.save(out / 'metadata.json', metadata)
results = {}; warnings = []
comparisons = [('arithmetic', 'candidate'), ('gcc', 'candidate'),
               ('clang', 'candidate'), ('candidate', 'candidate')]
for mode in suite.MODES:
    flags = suite.flags(source, mode)
    driver = out / ('driver-' + mode + '.o')
    subprocess.run([str(compilers['gcc']), *flags, '-c', str(root / 'benchmarks/simdjson/parse.cpp'), '-o', str(driver)], check=True)
    exes = {}
    for label, compiler in compilers.items():
        obj = out / (mode + '-' + label + '.o')
        command = [str(compiler), *flags, '-c', str(source / 'src/simdjson.cpp'), '-o', str(obj)]
        timing.retain(out / 'commands.jsonl', dict(command=command))
        with (out / (mode + '-' + label + '.log')).open('w') as log:
            subprocess.run(command, stdout=log, stderr=log, check=True)
        exe = out / ('parse-' + mode + '-' + label)
        subprocess.run([str(compilers['gcc']), str(driver), str(obj), '-pthread', '-o', str(exe)], check=True)
        exes[label] = exe
    for name in suite.INPUTS:
        path = source / 'jsonexamples' / name
        hashes = {}
        for label in labels:
            data = subprocess.check_output([str(exes[label]), str(path), 'check', '0'])
            check = out / ('check-' + mode + '-' + label + '-' + name)
            check.write_bytes(data); hashes[label] = timing.sha(check)
        if len(set(hashes.values())) != 1:
            raise ValueError('canonical JSON mismatch: ' + mode + '-' + name)
        repeats = 32
        def sample(label):
            value = json.loads(subprocess.check_output([str(exes[label]), str(path), 'time', str(repeats)], text=True))
            assert value['iterations'] == repeats and value['bytes'] == path.stat().st_size
            return value
        while sample('gcc')['cpu'] < 0.25:
            repeats *= 2
            assert repeats <= 1048576
        for label in labels:
            value = sample(label)
            timing.retain(out / 'warmups.jsonl', dict(value, mode=mode, input=name, label=label))
        for ref, candidate in comparisons:
            def one(label, phase, block, position):
                value = sample(label)
                timing.retain(out / 'observations.jsonl', dict(value, mode=mode, input=name,
                              reference=ref, candidate=candidate, label=label, phase=phase,
                              block=block, position=position))
                return value
            values = {}
            for phase, a, b, blocks in [('AA-before', ref, ref, 2),
                                       ('AB', ref, candidate, 5), ('AA-after', ref, ref, 2)]:
                rows = timing.observations(one, a, b, blocks, phase=phase)
                values[phase] = {key: suite.summarize(rows, key) for key in ('cpu', 'wall')}
            key = '-'.join([mode, name, ref, candidate])
            results[key] = values
            for phase in ('AA-before', 'AA-after'):
                for metric in ('cpu', 'wall'):
                    if abs(values[phase][metric]['speed'] - 1) > .02:
                        warnings.append(key + ' ' + phase + ' ' + metric)
            suite.save(out / 'results.json', dict(comparisons=results, calibration_warnings=warnings))
            print(key, values['AB']['cpu']['speed'], flush=True)
lines = ['# Controlled simdjson optimization probe', '',
         'Higher is better: reference CPU time / candidate CPU time. Paired ABBA/BAAB medians; all observations retained.', '',
         '| Backend | Input | Reference | Candidate | CPU ratio | Wall ratio |',
         '|---|---|---|---|---:|---:|']
for key, values in results.items():
    mode, name, ref, candidate = key.split('-')
    lines.append(f"| {mode} | {name} | {ref} | {candidate} | {values['AB']['cpu']['speed']:.4f} | {values['AB']['wall']['speed']:.4f} |")
lines += ['', 'A/A warnings above 2%: ' + (', '.join(warnings) if warnings else 'none'), '',
          'arithmetic: 963dbc3c8; candidate adds bounded bit projection and vector mask range facts.',
          'All four libraries produce identical canonical JSON for both inputs in each backend.']
(out / 'details.md').write_text('\n'.join(lines) + '\n')

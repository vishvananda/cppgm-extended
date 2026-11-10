#!/usr/bin/env python3
"""Record immutable index-c runs, preserving stdout, counters and command identity."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import time


def run(out, label, binary, cpus, selected=None):
    binary = Path(binary).resolve()
    before = hashlib.sha256(binary.read_bytes()).hexdigest()
    prefix = out / label
    command = ['perf', 'stat', '-x', '\t', '-o', str(prefix)+'.perf',
               '-e', 'cycles:u,instructions:u,branches:u,branch-misses:u,task-clock',
               '--', 'taskset', '-c', cpus, str(binary)]
    if selected:
        command.append(selected)
    env = os.environ.copy()
    env.pop('TEST', None)
    env['LC_ALL'] = 'C'
    start = time.monotonic()
    with open(str(prefix)+'.stdout', 'w') as stdout, open(str(prefix)+'.stderr', 'w') as stderr:
        result = subprocess.run(command, stdout=stdout, stderr=stderr, env=env)
    elapsed = time.monotonic()-start
    after = hashlib.sha256(binary.read_bytes()).hexdigest()
    text = Path(str(prefix)+'.stdout').read_text()
    rows = re.findall(r'^([^\n:]+(?:::[^\n:]+)*): (OK|ERR[^\n]*) in ([\d.]+)s$', text, re.M)
    summary = re.search(r'^Summary:\s*([\d.]+)s,\s*(\d+),\s*(\d+),\s*(\d+)', text, re.M)
    record = dict(label=label, binary=str(binary), sha256=before, command=command,
                  elapsed=elapsed, exit_code=result.returncode,
                  results=[dict(name=name, status=status, seconds=float(seconds)) for name,status,seconds in rows],
                  summary=list(summary.groups()) if summary else None)
    Path(str(prefix)+'.json').write_text(json.dumps(record, indent=2)+'\n')
    assert before == after, 'binary changed during run'
    assert result.returncode == 0 and summary and int(summary[4]) == 0, record
    assert int(summary[2]) == int(summary[3]) == len(rows), record
    if not selected:
        assert len(rows) == 50, record
    print(label, summary[1], 'seconds', len(rows), 'checks passed', flush=True)
    return record


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cpus', required=True)
    parser.add_argument('--case')
    parser.add_argument('runs', nargs='+', help='LABEL=IMMUTABLE_BINARY')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    for item in args.runs:
        label, binary = item.split('=', 1)
        run(args.output, label, binary, args.cpus, args.case)


if __name__ == '__main__':
    main()

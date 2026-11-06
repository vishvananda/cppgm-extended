#!/usr/bin/env python3
"""Inventory and resume a declared GCC C++11 native-dg execution cohort.

All inventory candidates get per-file accounting, including unselected inputs
and interfaces that still need adapters. This does not prove applicability or
language conformance. Each worker retains the single-case adapter's evidence.
"""
import argparse
from collections import Counter
from concurrent.futures import FIRST_COMPLETED, ThreadPoolExecutor, wait
import csv
import fcntl
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import signal
import subprocess
import sys
import time
import threading

from upstream_dg_proxy import sha256, write_json


SCRIPTS = ['run_upstream_dg_campaign.py', 'run_upstream_dg.py', 'upstream_dg_proxy.py']
SOURCE_SUFFIXES = {'.C', '.c', '.cc', '.cpp', '.cxx'}
ENVIRONMENT = ['PATH', 'LD_LIBRARY_PATH', 'CPATH', 'CPLUS_INCLUDE_PATH', 'LIBRARY_PATH',
               'CPPGM_HOST_CXX', 'CPPGM_STDLIB_FLAGS']


def selection(path, maximum=None):
    if path is None:
        return None
    chosen = set()
    with path.open(newline='') as stream:
        for row in csv.DictReader(stream):
            if maximum is not None and int(row['observation_id'].split('-')[1]) > maximum:
                continue
            chosen.add(row['source'])
    return chosen


def driver_for(row, sources):
    """Mirror dg.exp discovery boundaries, without pretending to run other .exp files."""
    if row['repository'] != 'gcc':
        return None, 'Clang lit/RUN adapter unavailable; C++11 applicability unresolved'
    path = Path(row['path'])
    if path.suffix not in SOURCE_SUFFIXES:
        return None, 'input kind requires a separate native-driver adapter'
    parts = path.parts
    if parts[:3] != ('gcc', 'testsuite', 'g++.dg'):
        if parts[:3] == ('gcc', 'testsuite', 'c-c++-common') and (
                len(parts) == 4 or (len(parts) == 5 and parts[3] == 'cpp')):
            return 'gcc/testsuite/g++.dg/dg.exp', None
        return None, 'specialized GCC family/driver not yet adapted: ' + row.get('family', '')
    native = sources / 'gcc'
    directory = path.parent
    while len(directory.parts) >= 3:
        drivers = sorted(p.relative_to(native).as_posix() for p in (native / directory).glob('*.exp'))
        if drivers:
            if directory == Path('gcc/testsuite/g++.dg') and drivers == ['gcc/testsuite/g++.dg/dg.exp']:
                # The original recursive dg.exp discovers only *.C here.
                if path.suffix != '.C':
                    return None, 'ordinary g++.dg driver does not discover this suffix'
                return drivers[0], None
            return ', '.join(drivers), 'specialized native .exp driver not yet adapted'
        directory = directory.parent
    return None, 'native driver ownership unresolved'


def make_plan(inventory, sources, chosen, dialects, modes):
    accounting, tasks, seen = [], [], set()
    with inventory.open() as stream:
        for line in stream:
            row = json.loads(line)
            if row['role'] != 'candidate':
                continue
            identity = row['repository'] + '/' + row['path']
            if identity in seen:
                raise ValueError('duplicate inventory identity: ' + identity)
            seen.add(identity)
            wanted = chosen is None or identity in chosen
            driver, blocker = driver_for(row, sources)
            case = dict(identity=identity, revision=row['revision'], sha256=row['sha256'],
                        driver=driver, applicability='unresolved', selected=wanted)
            if not wanted:
                case.update(status='not-scheduled-in-this-cohort', reason=blocker)
            elif blocker:
                case.update(status='harness-blocked', reason=blocker)
            else:
                source = sources / identity
                if not source.is_file() or sha256(source) != row['sha256']:
                    raise ValueError('source differs from pinned inventory: ' + identity)
                case.update(status='scheduled-native-variants', reason=None)
                for dialect in dialects:
                    for mode in modes:
                        task = dict(identity=identity, revision=row['revision'], sha256=row['sha256'],
                                    dialect=dialect, mode=mode, source=str(source.resolve()))
                        task['id'] = hashlib.sha256(json.dumps(task, sort_keys=True).encode()).hexdigest()[:24]
                        tasks.append(task)
            accounting.append(case)
    for identity in sorted((chosen or set()) - seen):
        accounting.append(dict(identity=identity, selected=True, status='selection-unmatched',
                               applicability='unresolved', reason='not a candidate in the pinned inventory'))
    return accounting, tasks


def process_stamp(pid):
    """Linux start ticks prevent a stale PID file from attaching to a reused PID."""
    try:
        fields = Path('/proc/' + str(pid) + '/stat').read_text().rsplit(')', 1)[1].split()
        if fields[0] == 'Z':
            return None
        return fields[19]
    except FileNotFoundError:
        return None


def worker_alive(state):
    pid, stamp = state.get('pid'), state.get('process_start_ticks')
    return pid is not None and stamp is not None and process_stamp(pid) == stamp


def result_for(task, directory, config, returncode=None):
    result = directory / 'native' / 'result.json'
    if not result.is_file():
        return dict(status='harness-incomplete', reason='worker terminated without a native result',
                    worker_returncode=returncode)
    try:
        data = json.loads(result.read_text())
        manifest = json.loads((result.parent / 'manifest.json').read_text())
        native_config = json.loads((result.parent / 'config.json').read_text())
        if (manifest['source']['sha256'] != task['sha256'] or
                manifest['source']['path'] != task['source'] or
                native_config['dialect'] != task['dialect'] or native_config['mode'] != task['mode'] or
                native_config['defaults'] != config['defaults'] or
                manifest['native_helpers'] != config['native_helpers'] or
                manifest['dejagnu_helpers'] != config['dejagnu_helpers'] or
                any(manifest['binaries'][name]['sha256'] != config['hashes'][name]
                    for name in ['candidate', 'host', 'runtest']) or
                sorted(manifest['adapters'].values()) != sorted(config['scripts'][name]
                    for name in ['run_upstream_dg.py', 'upstream_dg_proxy.py'])):
            raise ValueError('native result identity mismatch')
        if (not isinstance(data['status'], str) or not isinstance(data['reasons'], list) or
                not isinstance(data['native_counts'], dict)):
            raise ValueError('invalid native disposition')
    except (OSError, ValueError, KeyError) as error:
        return dict(status='harness-invalid-evidence', reason=str(error), worker_returncode=returncode)
    return dict(status=data['status'], reasons=data['reasons'], native_counts=data['native_counts'],
                result=str(result), result_sha256=sha256(result),
                manifest_sha256=sha256(result.parent / 'manifest.json'),
                config_sha256=sha256(result.parent / 'config.json'), worker_returncode=returncode)


def run_task(task, output, config):
    directory = output / 'cells' / task['id']
    directory.mkdir(exist_ok=True)
    state_path = directory / 'state.json'
    if state_path.exists():
        state = json.loads(state_path.read_text())
        if state['task'] != task:
            raise ValueError('existing task identity mismatch')
        if state['state'] == 'terminal':
            if 'result' in state:
                result = Path(state['result'])
                for path, digest in [(result, state['result_sha256']),
                                     (result.parent / 'manifest.json', state['manifest_sha256']),
                                     (result.parent / 'config.json', state['config_sha256'])]:
                    if sha256(path) != digest:
                        raise ValueError('recorded result changed: ' + task['id'])
            return state
        # The campaign lock proves the previous scheduler is gone. The worker
        # can still be alive; wait for that exact PID/start-time identity.
        while worker_alive(state):
            time.sleep(0.2)
        state.update(result_for(task, directory, config), state='terminal', recovered=True)
        write_json(state_path, state)
        return state
    if any(directory.iterdir()):
        raise ValueError('unowned existing cell artifacts: ' + str(directory))
    state = dict(task=task, state='starting', created_at_ns=time.time_ns())
    write_json(state_path, state)
    command = [sys.executable, '-B', str(output / 'adapter' / 'run_upstream_dg.py'),
               '--testsuite', config['testsuite'], '--dejagnu-library', config['dejagnu_library'],
               '--runtest', config['runtest'], '--candidate', str(output / 'candidate'),
               '--host', config['host'], '--case', task['source'], '--source-root', config['testsuite'],
               '--dialect', task['dialect'], '--mode', task['mode'], '--defaults', config['defaults'],
               '--timeout-seconds', str(config['timeout_seconds']),
               '--case-timeout-seconds', str(config['case_timeout_seconds']),
               '--cpu-seconds', str(config['cpu_seconds']),
               '--address-space-bytes', str(config['address_space_bytes']),
               '--file-bytes', str(config['file_bytes']), '--output', str(directory / 'native')]
    write_json(directory / 'command.json', command)
    with (directory / 'worker.stdout').open('wb') as out, (directory / 'worker.stderr').open('wb') as err:
        process = subprocess.Popen(command, stdout=out, stderr=err)
        state.update(state='running', pid=process.pid, process_start_ticks=process_stamp(process.pid))
        write_json(state_path, state)
        returncode = process.wait()
    state.update(result_for(task, directory, config, returncode), state='terminal')
    write_json(state_path, state)
    return state


def inputs(args):
    value = {name: str(getattr(args, name).resolve()) for name in
             ['inventory', 'sources', 'dejagnu_library']}
    value['testsuite'] = str(args.sources.resolve() / 'gcc/gcc/testsuite')
    value['selection_csv'] = str(args.selection_csv.resolve()) if args.selection_csv else None
    value.update(max_observation_id=args.max_observation_id, defaults=args.defaults,
                 dialects=args.dialects, modes=args.modes, timeout_seconds=args.timeout_seconds,
                 case_timeout_seconds=args.case_timeout_seconds, cpu_seconds=args.cpu_seconds,
                 address_space_bytes=args.address_space_bytes, file_bytes=args.file_bytes)
    for name in ['candidate', 'host', 'runtest']:
        found = shutil.which(getattr(args, name))
        if not found:
            raise ValueError('executable not found: ' + getattr(args, name))
        value[name] = str(Path(found).resolve())
    value['hashes'] = {name: sha256(value[name]) for name in ['inventory', 'candidate', 'host', 'runtest']}
    if args.selection_csv:
        value['hashes']['selection_csv'] = sha256(args.selection_csv)
    scripts = Path(__file__).resolve().parent
    value['scripts'] = {name: sha256(scripts / name) for name in SCRIPTS}
    value['native_helpers'] = {str(p.relative_to(Path(value['testsuite']))): sha256(p)
                               for p in (Path(value['testsuite']) / 'lib').glob('*.exp') if p.is_file()}
    value['driver_files'] = {str(p.relative_to(Path(value['testsuite']))): sha256(p)
                             for p in (Path(value['testsuite']) / 'g++.dg').rglob('*.exp') if p.is_file()}
    value['dejagnu_helpers'] = {str(p.relative_to(args.dejagnu_library)): sha256(p)
                              for p in args.dejagnu_library.rglob('*.exp') if p.is_file()}
    value['environment'] = {name: os.environ.get(name) for name in ENVIRONMENT}
    value['python'] = dict(executable=sys.executable, version=platform.python_version())
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ['inventory', 'sources', 'dejagnu-library', 'output']:
        parser.add_argument('--' + name, type=Path, required=True)
    for name in ['candidate', 'runtest']:
        parser.add_argument('--' + name, required=True)
    parser.add_argument('--host', default='g++')
    parser.add_argument('--selection-csv', type=Path)
    parser.add_argument('--max-observation-id', type=int)
    parser.add_argument('--dialects', nargs='+', choices=['c++11', 'gnu++11'], default=['c++11', 'gnu++11'])
    parser.add_argument('--modes', nargs='+', choices=['host', 'candidate'], default=['host', 'candidate'])
    parser.add_argument('--defaults', choices=['native', 'adapted'], default='native')
    parser.add_argument('--jobs', type=int, default=4)
    parser.add_argument('--timeout-seconds', type=float, default=15)
    parser.add_argument('--case-timeout-seconds', type=float, default=90)
    parser.add_argument('--cpu-seconds', type=int, default=30)
    parser.add_argument('--address-space-bytes', type=int, default=1073741824)
    parser.add_argument('--file-bytes', type=int, default=67108864)
    parser.add_argument('--resume', action='store_true')
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    if any(getattr(args, name) <= 0 for name in ['jobs', 'timeout_seconds', 'case_timeout_seconds',
                                               'cpu_seconds', 'address_space_bytes', 'file_bytes']):
        parser.error('jobs and resource limits must be positive')
    if len(set(args.dialects)) != len(args.dialects) or len(set(args.modes)) != len(args.modes):
        parser.error('duplicate variant selections')
    output = args.output.resolve()
    if not args.resume:
        output.mkdir(parents=True, exist_ok=False)
    elif not (output / 'manifest.json').is_file():
        parser.error('resume requires an existing campaign manifest')
    with (output / 'scheduler.lock').open('a') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            parser.error('another scheduler holds this campaign lock')
        config = inputs(args)
        if args.resume:
            saved = json.loads((output / 'manifest.json').read_text())
            if saved['inputs'] != config:
                parser.error('campaign inputs changed; create a new campaign')
            if sha256(output / 'candidate') != config['hashes']['candidate']:
                parser.error('frozen candidate changed')
            for name, digest in config['scripts'].items():
                if sha256(output / 'adapter' / name) != digest:
                    parser.error('frozen adapter changed: ' + name)
            for name in ['accounting.jsonl', 'tasks.jsonl']:
                if sha256(output / name) != saved['plan_hashes'][name]:
                    parser.error('campaign plan changed: ' + name)
            with (output / 'tasks.jsonl').open() as stream:
                tasks = [json.loads(s) for s in stream]
            # Resume revalidates every scheduled source, not only failed cells.
            for task in tasks:
                if sha256(task['source']) != task['sha256']:
                    parser.error('pinned source changed: ' + task['identity'])
        else:
            chosen = selection(args.selection_csv, args.max_observation_id)
            accounting, tasks = make_plan(args.inventory, args.sources.resolve(), chosen, args.dialects, args.modes)
            for name, rows in [('accounting.jsonl', accounting), ('tasks.jsonl', tasks)]:
                with (output / name).open('w') as stream:
                    for row in rows:
                        stream.write(json.dumps(row) + '\n')
            (output / 'adapter').mkdir()
            (output / 'cells').mkdir()
            shutil.copyfile(config['candidate'], output / 'candidate')
            (output / 'candidate').chmod(0o555)
            for name in SCRIPTS:
                shutil.copyfile(Path(__file__).with_name(name), output / 'adapter' / name)
            saved = dict(inputs=config, plan_hashes={name: sha256(output / name)
                         for name in ['accounting.jsonl', 'tasks.jsonl']},
                         accounting_counts=dict(Counter(row['status'] for row in accounting)),
                         selected_cases=sum(row['selected'] for row in accounting), tasks=len(tasks),
                         scope='single native variants; applicability unresolved; full original matrix not claimed')
            write_json(output / 'manifest.json', saved)
        counts, done = Counter(), 0
        write_json(output / 'progress.json', dict(state='prepared', tasks=len(tasks), completed=0))
        if args.prepare_only:
            print(json.dumps(saved['accounting_counts']), flush=True)
            return 0
        started = time.monotonic()
        stop = threading.Event()
        def stop_scheduling(number, frame):
            stop.set()
        for number in (signal.SIGINT, signal.SIGTERM):
            signal.signal(number, stop_scheduling)
        pending, iterator, exhausted = set(), iter(tasks), False
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            while pending or (not exhausted and not stop.is_set()):
                while len(pending) < args.jobs and not exhausted and not stop.is_set():
                    task = next(iterator, None)
                    if task is None:
                        exhausted = True
                    else:
                        pending.add(pool.submit(run_task, task, output, config))
                if not pending:
                    break
                finished, pending = wait(pending, timeout=1, return_when=FIRST_COMPLETED)
                for future in finished:
                    state = future.result()
                    counts[state['status']] += 1
                    done += 1
                    progress = dict(state='draining' if stop.is_set() else 'running', tasks=len(tasks),
                                    completed=done, counts=dict(counts), elapsed_seconds=time.monotonic() - started)
                    write_json(output / 'progress.json', progress)
                    if done % 20 == 0 or done == len(tasks):
                        print(json.dumps(progress), flush=True)
        progress = dict(state='interrupted' if stop.is_set() else 'terminal', tasks=len(tasks), completed=done, counts=dict(counts),
                        elapsed_seconds=time.monotonic() - started, full_upstream_pass=False,
                        goal_complete=False, accounting_counts=saved['accounting_counts'])
        write_json(output / 'progress.json', progress)
        print(json.dumps(progress), flush=True)
    return 130 if stop.is_set() else 0


if __name__ == '__main__':
    sys.exit(main())

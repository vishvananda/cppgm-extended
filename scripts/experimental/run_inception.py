#!/usr/bin/env python3
"""Run the optional Linux x86-64 WASI and Chromium inception lane."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import platform
import signal
import subprocess
import sys
import time
import uuid

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent


def arguments(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--directory', type=Path,
                        help='new frozen package (default: unique directory in obj/wasm-inception)')
    parser.add_argument('--tools', type=Path, default=REPO / 'obj/wasm-tools')
    parser.add_argument('--compiler', type=Path, default=REPO / 'dev/cppgm++')
    parser.add_argument('--playwright', type=Path, default=HERE / 'node_modules/playwright')
    parser.add_argument('--shim', type=Path, default=HERE / 'node_modules/@bjorn3/browser_wasi_shim')
    parser.add_argument('--chromium', type=Path, help='override the pinned Playwright browser')
    parser.add_argument('--node', default='node')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--timeout', type=int, default=7200, help='seconds per local phase')
    args = parser.parse_args(argv)
    if args.jobs < 1 or args.timeout < 1:
        parser.error('--jobs and --timeout must be positive')
    return args


def plan(args, package):
    tool = [sys.executable, str(HERE / 'wasi_inception.py')]
    common = ['--directory', str(package)]
    runtime = ['--wasmtime', str(args.tools.resolve() / 'bin/wasmtime'), '--jobs', str(args.jobs)]
    return [
        ('prepare', tool + ['prepare', *common, '--compiler', str(args.compiler.resolve()),
                           '--sdk', str(args.tools.resolve() / 'wasi-sdk'),
                           '--shim', str(args.shim.resolve()), '--x86-headers']),
        ('wasi', tool + ['build', *common, *runtime]),
        ('x86', tool + ['x86', *common, *runtime, '--levels', '0', '2', '3']),
        ('chromium', [args.node, str(HERE / 'wasi_browser_test.cjs'), str(package),
                      str(args.playwright.resolve()), str(args.chromium.resolve()) if args.chromium else '',
                      '', 'chromium']),
    ]


def preflight(args):
    if platform.system() != 'Linux' or platform.machine() not in ('x86_64', 'AMD64'):
        raise RuntimeError('this optional host profile requires Linux x86-64')
    for path in [args.compiler, args.tools / 'bin/wasmtime', args.tools / 'wasi-sdk/VERSION',
                 args.shim / 'dist/index.js', args.playwright / 'package.json',
                 REPO / 'obj/generated/cppgm_builtin_host_config.h']:
        if not path.is_file():
            raise RuntimeError(f'missing prerequisite: {path}; see scripts/experimental/README.md')
    shim = json.loads((args.shim / 'package.json').read_text())
    if (shim.get('name'), shim.get('version')) != ('@bjorn3/browser_wasi_shim', '0.4.2'):
        raise RuntimeError('expected browser_wasi_shim 0.4.2')
    subprocess.run([args.node, '-e',
                    'const p=require(process.argv[1]);require("fs").accessSync(process.argv[2] || p.chromium.executablePath());',
                    str(args.playwright.resolve()), str(args.chromium.resolve()) if args.chromium else ''],
                   check=True)


def invoke(command, log, timeout):
    """Retain diagnostics and terminate our process group on failure/cancellation."""
    with log.open('wb') as output:
        process = subprocess.Popen(command, cwd=REPO, stdout=output, stderr=subprocess.STDOUT,
                                   start_new_session=True)
        try:
            return process.wait(timeout=timeout)
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()


def run(args):
    preflight(args)
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + uuid.uuid4().hex[:8]
    package = (args.directory or REPO / 'obj/wasm-inception' / stamp).resolve()
    if package.exists() or package.with_name(package.name + '-logs').exists():
        raise RuntimeError(f'use a new package directory: {package}')
    logs = package.with_name(package.name + '-logs')
    logs.mkdir(parents=True)
    records = []
    print(f'Frozen package: {package}\nLogs: {logs}', flush=True)
    try:
        for name, command in plan(args, package):
            print(f'Running {name} (log: {logs / (name + ".log")})', flush=True)
            row = {'phase': name, 'command': command, 'exit': None}
            records.append(row)
            started = time.monotonic()
            try:
                row['exit'] = invoke(command, logs / (name + '.log'), args.timeout)
            finally:
                row['seconds'] = time.monotonic() - started
            if row['exit']:
                raise RuntimeError(f'{name} failed (exit {row["exit"]}); see {logs / (name + ".log")}')
    finally:
        (logs / 'results.json').write_text(json.dumps(records, indent=2) + '\n')
    print('PASS: Wasmtime and Chromium W1/W2/W3 bytes match; native/WASI x86 O0/O2/O3 match.', flush=True)


if __name__ == '__main__':
    try:
        run(arguments())
    except (RuntimeError, subprocess.SubprocessError) as error:
        sys.exit(str(error))

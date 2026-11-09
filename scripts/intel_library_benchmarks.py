#!/usr/bin/env python3
"""Reuse the normal library producers and publishers on the dedicated Intel host."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import urllib.request

SCRIPTS = {
    'index-cpp': 'index_cpp_ci.py',
    'simdjson': 'simdjson_ci.py',
    'godot': 'godot_ci.py',
    'boost': 'boost_benchmarks.py',
    'internal': 'internal_cpp_benchmarks.py',
}
DIRECTORY = Path(__file__).resolve().parent


def run_command(suite, source, jobs):
    return [sys.executable, str(DIRECTORY / SCRIPTS[suite]), 'run',
            '--compiler', str(source / 'dev/cppgm++'), '--repo-root', str(source),
            '--output', str(source / 'obj' / suite)] + (
                ['--jobs', str(jobs)] if suite in ('godot', 'boost', 'internal') else [])


def run(args):
    command = run_command(args.suite, args.source.resolve(strict=True), args.jobs)
    if args.suite != 'boost':
        subprocess.run(command, check=True)
        return
    # The regular producer verifies the pinned archive hash before extraction.
    with tempfile.TemporaryDirectory(prefix='cppgm-boost-') as temporary:
        archive = Path(temporary) / 'boost_1_92_0.tar.bz2'
        url = 'https://archives.boost.io/release/1.92.0/source/' + archive.name
        with urllib.request.urlopen(url, timeout=120) as response, archive.open('wb') as stream:
            shutil.copyfileobj(response, stream)
        subprocess.run(command + ['--archive', str(archive)], check=True)


def collect(source, target):
    """Keep diagnostics and observations, excluding source trees and binaries."""
    suffixes = {'.json', '.jsonl', '.xml', '.log', '.jam', '.hpp', '.md',
                '.stdout', '.stderr', '.time', '.perf'}
    for path in source.rglob('*'):
        relative = path.relative_to(source)
        if ({'build', 'source'}.intersection(relative.parts) or not path.is_file()
                or not (path.suffix in suffixes or path.name in ('stdout', 'stderr')
                        or path.name.endswith('probe'))):
            continue
        destination = target / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--suite', choices=SCRIPTS, required=True)
    runner.add_argument('--source', type=Path, required=True)
    runner.add_argument('--jobs', type=int, default=16)
    collector = commands.add_parser('collect')
    collector.add_argument('source', type=Path)
    collector.add_argument('target', type=Path)
    publisher = commands.add_parser('publish')
    publisher.add_argument('--suite', choices=SCRIPTS, required=True)
    publisher.add_argument('--artifact', type=Path, required=True)
    publisher.add_argument('--target', type=Path, required=True)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    args = parser.parse_args()
    if args.command == 'run':
        if args.jobs < 1:
            parser.error('--jobs must be positive')
        run(args)
    elif args.command == 'collect':
        collect(args.source, args.target)
    else:
        subprocess.run([sys.executable, str(DIRECTORY / SCRIPTS[args.suite]), 'publish',
                        str(args.artifact), str(args.target), '--revision', args.revision,
                        '--run-url', args.run_url, '--platform', 'intel'], check=True)


if __name__ == '__main__':
    main()

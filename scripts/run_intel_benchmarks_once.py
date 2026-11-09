#!/usr/bin/env python3
"""Run trusted Intel benchmarks sequentially, with one temporary runner per job."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tarfile
import tempfile
import urllib.request

SUITES = {
    'public': 'public-benchmarks.yml',
    'self-build': 'self-build-benchmarks.yml',
    'index-cpp': 'index-cpp.yml',
    'simdjson': 'simdjson.yml',
    'godot': 'godot.yml',
    'boost': 'boost.yml',
}


def github(*arguments):
    return json.loads(subprocess.check_output(['gh', 'api', *arguments], text=True))


def run_suites(args, revision, directory):
    """An ephemeral runner accepts one job, so give each suite its own runner."""
    for suite in SUITES:
        command = [sys.executable, str(Path(__file__).resolve()), '--suite', suite,
                   '--repo', args.repo, '--ref', args.ref, '--revision', revision,
                   '--directory', str(directory / suite)]
        if suite == 'public' and args.course_quality:
            command.append('--course-quality')
        subprocess.run(command, check=True)


def dispatch_command(args, revision, label):
    workflow = SUITES[args.suite]
    command = ['gh', 'workflow', 'run', workflow, '--repo', args.repo,
               '--ref', args.ref, '-f', 'platform=intel', '-f', f'revision={revision}',
               '-f', f'runner_label={label}']
    if args.course_quality and args.suite == 'public':
        command += ['-f', 'course_quality=true']
    return command


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', default='vishvananda/cppgm-extended')
    parser.add_argument('--ref', default='main', help='revision containing the workflow')
    parser.add_argument('--revision', default='main', help='trusted compiler revision to measure')
    parser.add_argument('--directory', type=Path, help='new directory for runner files and logs')
    parser.add_argument('--listen', action='store_true', help='accept one manually queued job instead of dispatching')
    parser.add_argument('--course-quality', action='store_true', help='also measure the optional course exercises')
    parser.add_argument('--suite', choices=['all', *SUITES], default='all',
                        help='run all six suites sequentially (default), or just one')
    parser.add_argument('--plan', action='store_true', help='show the pinned campaign without registering runners or starting jobs')
    args = parser.parse_args()
    revision = github(f'repos/{args.repo}/commits/{args.revision}')['sha']
    if args.plan:
        suites = SUITES if args.suite == 'all' else {args.suite: SUITES[args.suite]}
        print(json.dumps(dict(revision=revision, workflow_ref=args.ref, suites=suites,
                              execution='sequential; one temporary runner per suite'), indent=2))
        return
    if not Path('/proc/cpuinfo').exists() or 'GenuineIntel' not in Path('/proc/cpuinfo').read_text():
        parser.error('this helper requires Intel Linux hardware')
    if not shutil.which('podman') and not shutil.which('docker'):
        parser.error('install Podman or Docker to run the Ubuntu benchmark environment')
    directory = args.directory or Path(tempfile.mkdtemp(prefix='cppgm-intel-runner-'))
    if args.directory:
        directory.mkdir(mode=0o700, parents=True, exist_ok=False)
    directory = directory.resolve()
    if args.suite == 'all' and not args.listen:
        run_suites(args, revision, directory)
        return
    label = 'cppgm-intel-' + hashlib.sha256(str(directory).encode()).hexdigest()[:16]
    name = label
    release = github('repos/actions/runner/releases/latest')
    asset = next(a for a in release['assets'] if a['name'].startswith('actions-runner-linux-x64-'))
    expected = asset.get('digest') or ''
    if not expected.startswith('sha256:'):
        raise RuntimeError('the official runner release has no SHA-256 asset digest')
    archive = directory / asset['name']
    with urllib.request.urlopen(asset['browser_download_url']) as source, archive.open('wb') as target:
        shutil.copyfileobj(source, target)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != expected.split(':', 1)[1]:
        raise RuntimeError('runner archive checksum mismatch')
    with tarfile.open(archive) as bundle:
        bundle.extractall(directory, filter='data')
    print(f'Runner files and logs: {directory}', flush=True)
    registered = False
    run_id = None
    try:
        token = github('--method', 'POST', f'repos/{args.repo}/actions/runners/registration-token')['token']
        with (directory / 'configure.log').open('wb') as log:
            result = subprocess.run([str(directory / 'config.sh'), '--unattended', '--ephemeral',
                '--disableupdate', '--url', f'https://github.com/{args.repo}', '--token', token,
                '--name', name, '--labels', f'cppgm-intel,{label}', '--work', '_work'],
                cwd=directory, stdout=log, stderr=log)
        if result.returncode:
            raise RuntimeError(f'runner registration failed; see {directory / "configure.log"}')
        registered = True
        if not args.listen:
            # Queue the job for this runner's unique label, not any other runner.
            output = subprocess.check_output(dispatch_command(args, revision, label), text=True)
            print(output, end='', flush=True)
            match = re.search(r'/actions/runs/(\d+)', output)
            if match:
                run_id = match[1]
            print(f'Queued Intel measurement of {revision}', flush=True)
        else:
            print('Listening for one job with runner_label=cppgm-intel', flush=True)
        with (directory / 'runner.log').open('wb') as log:
            process = subprocess.Popen([str(directory / 'run.sh')], cwd=directory,
                stdout=log, stderr=log, start_new_session=True)
            try:
                status = process.wait(timeout=10800)
            except BaseException:
                os.killpg(process.pid, signal.SIGINT)
                try:
                    process.wait(timeout=30)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
                raise
        if status:
            raise RuntimeError(f'runner exited {status}; see {directory / "runner.log"}')
        if not args.listen:
            if run_id is None:
                raise RuntimeError('gh did not return the run URL; check Actions before starting another suite')
            # A healthy runner process does not imply a successful benchmark.
            # Wait for validation and badge publication before the next suite.
            subprocess.run(['gh', 'run', 'watch', run_id, '--repo', args.repo, '--exit-status'], check=True)
    finally:
        try:
            if registered:
                runners = github(f'repos/{args.repo}/actions/runners')['runners']
                for runner in runners:
                    if runner['name'] == name:
                        subprocess.run(['gh', 'api', '--method', 'DELETE',
                            f'repos/{args.repo}/actions/runners/{runner["id"]}'], check=True)
        finally:
            # Keep measurements and diagnostic logs, but no runner credentials.
            for filename in ['.credentials', '.credentials_rsaparams', '.runner']:
                (directory / filename).unlink(missing_ok=True)


if __name__ == '__main__':
    main()

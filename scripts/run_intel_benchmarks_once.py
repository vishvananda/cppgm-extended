#!/usr/bin/env python3
"""Register a temporary Intel runner, dispatch one trusted benchmark, then remove it."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tarfile
import tempfile
import urllib.request


def github(*arguments):
    return json.loads(subprocess.check_output(['gh', 'api', *arguments], text=True))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', default='vishvananda/cppgm-extended')
    parser.add_argument('--ref', default='main', help='revision containing the workflow')
    parser.add_argument('--revision', default='main', help='trusted compiler revision to measure')
    parser.add_argument('--directory', type=Path, help='new directory for runner files and logs')
    parser.add_argument('--listen', action='store_true', help='accept one manually queued job instead of dispatching')
    args = parser.parse_args()
    if not Path('/proc/cpuinfo').exists() or 'GenuineIntel' not in Path('/proc/cpuinfo').read_text():
        parser.error('this helper requires Intel Linux hardware')
    if not shutil.which('podman') and not shutil.which('docker'):
        parser.error('install Podman or Docker to run the Ubuntu benchmark environment')
    revision = github(f'repos/{args.repo}/commits/{args.revision}')['sha']
    directory = args.directory or Path(tempfile.mkdtemp(prefix='cppgm-intel-runner-'))
    if args.directory:
        directory.mkdir(mode=0o700, parents=True, exist_ok=False)
    directory = directory.resolve()
    label = 'cppgm-intel-' + directory.name.removeprefix('cppgm-intel-runner-')
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
            subprocess.run(['gh', 'workflow', 'run', 'public-benchmarks.yml', '--repo', args.repo,
                '--ref', args.ref, '-f', 'platform=intel', '-f', f'revision={revision}',
                '-f', f'runner_label={label}'], check=True)
            print(f'Queued Intel measurement of {revision}', flush=True)
        else:
            print('Listening for one job with runner_label=cppgm-intel', flush=True)
        with (directory / 'runner.log').open('wb') as log:
            process = subprocess.Popen([str(directory / 'run.sh')], cwd=directory,
                stdout=log, stderr=log, start_new_session=True)
            try:
                status = process.wait(timeout=7200)
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

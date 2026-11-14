#!/usr/bin/env python3
"""Export the committed source of a version tag, never local build products."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import re
import subprocess


def export(root: Path, tag: str, destination: Path):
    # Match the repository's v8 / v7.2 style as well as patch and RC tags.
    if not re.fullmatch(r'v[0-9]+(?:\.[0-9]+){0,2}(?:-(?:rc|beta|alpha)[0-9]+)?', tag):
        raise ValueError('expected a version tag such as v8, v8.1 or v8.1.2-rc1')
    commit = subprocess.check_output(['git', 'rev-parse', '--verify',
                                      'refs/tags/' + tag + '^{commit}'], cwd=root, text=True).strip()
    head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
    if commit != head:
        raise ValueError('check out the tagged commit before exporting a release')
    # git archive reads the Git tree, including licenses and submodule pointers;
    # neither ignored objects nor working-tree/untracked files can enter it.
    destination.mkdir(parents=True, exist_ok=True)
    archive = destination / ('cppgm-source-' + tag + '.tar.gz')
    with archive.open('xb') as output:
        with gzip.GzipFile(filename='', mode='wb', fileobj=output, mtime=0) as compressed:
            process = subprocess.Popen(['git', 'archive', '--format=tar',
                                        '--prefix=cppgm-' + tag + '/', commit], cwd=root, stdout=subprocess.PIPE)
            try:
                while chunk := process.stdout.read(1024 * 1024):
                    compressed.write(chunk)
            finally:
                process.stdout.close()
            if process.wait():
                raise RuntimeError('git archive failed')
    provenance = dict(schema=1, tag=tag, source_sha=commit, archive=archive.name,
                      sha256=hashlib.sha256(archive.read_bytes()).hexdigest())
    with (destination / 'source-provenance.json').open('x') as output:
        output.write(json.dumps(provenance, indent=2) + '\n')
    return archive


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    print(export(Path(__file__).resolve().parents[1], args.tag, args.output))

#!/usr/bin/env python3
"""Keep the generated benchmark branch small; raw evidence belongs to CI artifacts."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import posixpath
import re
import subprocess

MAX_FILE_BYTES = 2 * 1024 * 1024
MAX_TREE_BYTES = 64 * 1024 * 1024
BOOST_SNAPSHOT = re.compile(r'(boost/(?:o3/)?(?:history/[0-9a-f]{40}/)?(?:epyc|intel|github-intel))/')
RUN_URL = re.compile(r'https://github.com/[^/\s)]+/[^/\s)]+/actions/runs/[0-9]+')


def retained(name):
    path = PurePosixPath(name)
    if path.suffix not in ('.md', '.svg', '.json') or path.name.startswith('commands-'):
        return False
    if 'runtime' in path.parts:
        return False
    snapshot = BOOST_SNAPSHOT.match(name)
    if snapshot:
        relative = name[snapshot.end():]
        return relative in ('metadata.json', 'results.json', 'report.md') or (
            relative.startswith(('standard/', 'suites/')) and path.suffix in ('.md', '.json'))
    return True


def check(directory):
    total, count = 0, 0
    for path in directory.rglob('*'):
        name = path.relative_to(directory)
        if '.git' in name.parts or not path.is_file():
            continue
        size = path.stat().st_size
        if path.is_symlink() or not retained(name.as_posix()) or size > MAX_FILE_BYTES:
            raise ValueError(f'not a compact published result: {name} ({size} bytes); use CI artifacts')
        total += size
        count += 1
    if total > MAX_TREE_BYTES:
        raise ValueError(f'published tree is {total} bytes, above {MAX_TREE_BYTES}; review retained reports')
    return dict(files=count, bytes=total)


def prepare(directory):
    # Whitespace is useful in the rendered tables, not in repeated machine
    # records. Keep the JSON schema and every value, with one canonical encoding.
    for path in directory.rglob('*.json'):
        if '.git' not in path.relative_to(directory).parts and retained(path.relative_to(directory).as_posix()):
            document = json.loads(path.read_text())
            metadata_path = (path.parent.parent if path.parent.name == 'standard' else path.parent) / 'metadata.json'
            if document.get('schema') == 1 and metadata_path.is_file():
                metadata = json.loads(metadata_path.read_text())
                for row in document.get('records', []) + document.get('validations', []):
                    provenance = row.get('provenance', {})
                    if provenance.get('metadata') == metadata:
                        del provenance['metadata']
                        provenance.update(metadata_sha256=hashlib.sha256(
                            json.dumps(metadata, sort_keys=True).encode()).hexdigest(),
                            metadata_url=row['details_url'].rsplit('/', 1)[0] + '/metadata.json')
            text = json.dumps(document, ensure_ascii=False, separators=(',', ':')) + '\n'
            if path.read_text() != text:
                path.write_text(text)
    return check(directory)


def compact_git(repository, revision, destination):
    """Materialize only reports from a historical tree, without checking out its raw logs.

    This prepares a new directory for review. It never changes Git refs or history.
    """
    if destination.exists() and any(destination.iterdir()):
        raise ValueError('compact output must be an empty directory')
    names = subprocess.check_output(['git', '-C', str(repository), 'ls-tree', '-rz', '--name-only', revision])
    names = {s.decode() for s in names.split(b'\0') if s}
    selected = {name for name in names if retained(name)}

    def read(name):
        return subprocess.check_output(['git', '-C', str(repository), 'show', revision + ':' + name])

    run_urls = {}
    for name in sorted(selected):
        if name.endswith('/standard/records.json'):
            document = json.loads(read(name))
            rows = document.get('records', []) + document.get('validations', [])
            if rows and RUN_URL.fullmatch(rows[0].get('run_url', '')):
                run_urls[name.removesuffix('/standard/records.json')] = rows[0]['run_url']
    for name in sorted(selected):
        snapshot = BOOST_SNAPSHOT.match(name)
        prefix = snapshot.group(1) if snapshot else None
        source = name
        if prefix and name == prefix + '/results.json':
            source = prefix + '/standard/records.json'
        data = read(source)
        if name.endswith('.md'):
            text = data.decode()
            own_url = RUN_URL.search(text)
            url = run_urls.get(prefix) or (own_url.group() if own_url else None)
            if url:
                def link(match):
                    target = match.group(1)
                    if '://' in target or target.startswith('#'):
                        return match.group()
                    resolved = posixpath.normpath(posixpath.join(posixpath.dirname(name), target.split('#')[0]))
                    exists = resolved in selected or any(p.startswith(resolved.rstrip('/') + '/') for p in selected)
                    return '(' + (target if exists else url) + ')'
                text = re.sub(r'(?<=\])\(([^()\s]+)\)', link, text)
            data = text.encode()
        elif name.endswith('.json') and prefix and name.endswith(('results.json', 'records.json')):
            document = json.loads(data)
            for row in document.get('records', []) + document.get('validations', []):
                row['raw_url'] = row['run_url']
            data = (json.dumps(document, indent=2) + '\n').encode()
        target = destination / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    return prepare(destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    validator = commands.add_parser('check')
    validator.add_argument('directory', type=Path)
    publisher = commands.add_parser('prepare')
    publisher.add_argument('directory', type=Path)
    migrate = commands.add_parser('compact-git')
    migrate.add_argument('repository', type=Path)
    migrate.add_argument('revision')
    migrate.add_argument('destination', type=Path)
    args = parser.parse_args()
    result = (check(args.directory) if args.command == 'check' else prepare(args.directory)
              if args.command == 'prepare' else compact_git(args.repository, args.revision, args.destination))
    print(json.dumps(result, sort_keys=True))


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Verify a frozen compiler bundle and unpack its virtual filesystem."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath


def digest(data):
    return hashlib.sha256(data).hexdigest()


def unpack(package: Path, output: Path):
    data = (package / 'manifest.json').read_bytes()
    if digest(data) != (package / 'manifest.sha256').read_text().strip():
        raise ValueError('manifest checksum mismatch')
    manifest = json.loads(data)
    bundle = (package / 'bundle.bin').read_bytes()
    if manifest.get('schema') != 1 or digest(bundle) != manifest.get('bundle_sha256'):
        raise ValueError('bundle checksum mismatch or unsupported schema')
    seen = set()
    for row in manifest['files']:
        name = row['path']
        path = PurePosixPath(name)
        offset, size = row['offset'], row['size']
        if (not name or not path.parts or path.is_absolute() or '..' in path.parts or '\\' in name
                or str(path) != name or name in seen):
            raise ValueError('invalid or duplicate input path')
        seen.add(name)
        if (type(offset) is not int or type(size) is not int or offset < 0 or size < 0
                or offset + size > len(bundle) or digest(bundle[offset:offset + size]) != row['sha256']):
            raise ValueError('input bounds or checksum mismatch: ' + name)
    # A new root prevents symlink traversal and accidental overwrite of local work.
    output.mkdir()
    for row in manifest['files']:
        path = output / row['path']
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open('xb') as target:
            target.write(bundle[row['offset']:row['offset'] + row['size']])
    return len(seen)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    print(f'Unpacked {unpack(args.package, args.output)} verified inputs')

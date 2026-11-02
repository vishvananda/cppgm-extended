#!/usr/bin/env python3
"""Update one CPU's badges while retaining the other CPU's last measurement."""
import argparse
import datetime
import json
from pathlib import Path
import re

BADGES = ('o0-compile', 'o0-generated', 'o3-compile', 'o3-generated',
          'self-compile', 'self-built')


def prepare(artifact, target, platform, revision, run_url):
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('the measured revision must be a full commit hash')
    artifact, target = Path(artifact), Path(target)
    environment = json.loads((artifact / 'results.json').read_text())['environment']
    cpu = environment.get('cpu') or ''
    expected = 'Intel' if platform == 'intel' else 'AMD EPYC'
    if expected not in cpu:
        raise ValueError(f'{platform} badges require {expected} hardware; measured {cpu!r}')
    # Validate the whole set before replacing any previous measurement.
    badges = {}
    for name in BADGES:
        data = json.loads((artifact / 'badges' / f'{name}.json').read_text())
        if data.get('schemaVersion') != 1 or not isinstance(data.get('message'), str):
            raise ValueError(f'invalid badge: {name}')
        badges[name] = data
    # Keep the original EPYC URLs working while the README uses CPU directories.
    destinations = [target / 'intel'] if platform == 'intel' else [target / 'epyc', target]
    provenance = dict(revision=revision, cpu=cpu, run_url=run_url,
                      published_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      environment=environment)
    badges['revision'] = dict(schemaVersion=1, label='commit', message=revision[:9], color='blue')
    for destination in destinations:
        destination.mkdir(parents=True, exist_ok=True)
        for name, data in badges.items():
            (destination / f'{name}.json').write_text(json.dumps(data, indent=1) + '\n')
        (destination / 'provenance.json').write_text(json.dumps(provenance, indent=1) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('artifact', type=Path)
    parser.add_argument('target', type=Path)
    parser.add_argument('--platform', choices=['epyc', 'intel'], required=True)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--run-url', required=True)
    args = parser.parse_args()
    prepare(args.artifact, args.target, args.platform, args.revision, args.run_url)


if __name__ == '__main__':
    main()

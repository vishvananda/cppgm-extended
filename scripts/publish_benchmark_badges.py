#!/usr/bin/env python3
"""Update one CPU's badges while retaining the other CPU's last measurement."""
import argparse
import datetime
import json
from pathlib import Path
import re

import public_benchmarks

BADGES = ('o0-compile', 'o0-generated', 'o3-compile', 'o3-generated',
          'self-compile', 'self-built')


def prepare(artifact, target, platform, revision, run_url, legacy_epyc=False):
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('the measured revision must be a full commit hash')
    artifact, target = Path(artifact), Path(target)
    document = json.loads((artifact / 'results.json').read_text())
    environment = document['environment']
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
    destinations = [target / platform]
    # Only main's automatic run updates the original, unqualified URLs.
    if platform == 'epyc' and legacy_epyc:
        destinations.append(target)
    provenance = dict(revision=revision, cpu=cpu, run_url=run_url,
                      published_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      environment=environment)
    badges['revision'] = dict(schemaVersion=1, label='commit', message=revision[:9], color='1e40af')
    _, count = public_benchmarks.mean_ratio(document['programs'], 'run_cpu_O3', 'gcc')
    badges['o3-programs'] = dict(schemaVersion=1, label='programs', message=str(count), color='475569')
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
    parser.add_argument('--legacy-epyc', action='store_true', help='also update main\'s original EPYC URLs')
    args = parser.parse_args()
    prepare(args.artifact, args.target, args.platform, args.revision, args.run_url, args.legacy_epyc)


if __name__ == '__main__':
    main()

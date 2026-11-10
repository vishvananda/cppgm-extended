#!/usr/bin/env python3
"""Collect every completed perf observation and generate the baseline table."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics


def counters(path):
    result = {}
    for line in path.read_text().splitlines():
        fields = line.split('\t')
        if len(fields) >= 3 and not line.startswith('#'):
            try:
                result[fields[2]] = float(fields[0])
            except ValueError:
                pass
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('data', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    observations = []
    for path in sorted(args.data.glob('*/*.json')):
        record = json.loads(path.read_text())
        if not isinstance(record, dict) or not record.get('command', [None])[0] == 'perf':
            continue
        record['source'] = str(path.relative_to(args.data))
        record['counters'] = counters(path.with_suffix('.perf'))
        record['raw_files'] = {}
        for extension in ('.stdout', '.stderr', '.perf'):
            raw = path.with_suffix(extension)
            record['raw_files'][str(raw.relative_to(args.data))] = hashlib.sha256(raw.read_bytes()).hexdigest()
        observations.append(record)
    args.output.mkdir(exist_ok=True)
    bundle = {'provenance': json.loads((args.data/'reproduction.json').read_text()),
              'raw_data_directory': str(args.data.resolve()), 'observations': observations}
    (args.output/'observations.json').write_text(json.dumps(bundle, indent=2)+'\n')
    by_source = {row['source']: row for row in observations}
    labels = ('a1', 'a2', 'b1', 'b2', 'gcc1')
    data = {label: {row['name']: row['seconds'] for row in
                   by_source['paired-full/'+label+'.json']['results']} for label in labels}
    rows = []
    for name in data['a1']:
        clang = statistics.mean(data[label][name] for label in ('a1', 'a2'))
        gm = statistics.mean(data[label][name] for label in ('b1', 'b2'))
        gcc = data['gcc1'][name]
        rows.append((gm-clang, name, gm, clang, gcc))
    lines = ['| Workload | GM seconds | Clang seconds | GCC seconds | GM vs Clang ↑ | GM vs GCC ↑ |',
             '|---|---:|---:|---:|---:|---:|']
    for _, name, gm, clang, gcc in sorted(rows, reverse=True):
        lines.append(f'| {name} | {gm:.4f} | {clang:.4f} | {gcc:.4f} | {clang/gm:.3f}× | {gcc/gm:.3f}× |')
    (args.output/'baseline-table.md').write_text('\n'.join(lines)+'\n')
    print(len(observations), 'completed observations collected')


if __name__ == '__main__':
    main()

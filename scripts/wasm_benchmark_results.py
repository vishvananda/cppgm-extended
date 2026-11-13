#!/usr/bin/env python3
"""Adapt checked Wasm measurements to the shared reports and compact publication."""
import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
import re
import shutil
import statistics
import tempfile

import benchmark_hosts
import benchmark_report
from library_benchmark_badges import HOSTS, geometric, svg

ROOT = Path(__file__).resolve().parents[1]
CASES = ('hash', 'filter', 'matrix', 'transform', 'sort', 'graph', 'json', 'lzss')


def load(artifacts):
    documents = [json.loads((path / 'results.json').read_text()) for path in artifacts]
    seen = set()
    for path, document in zip(artifacts, documents):
        metadata = document['metadata']
        if document['status'] != 'complete' or document['schema'] != 2:
            raise ValueError('only complete checked measurements can be reported')
        if json.loads((path / 'metadata.json').read_text()) != metadata:
            raise ValueError('result provenance differs from metadata')
        if not re.fullmatch('[0-9a-f]{40}', metadata['revision']):
            raise ValueError('missing compiler revision')
        if metadata['optimization'] in seen:
            raise ValueError('duplicate optimization profile')
        if not re.fullmatch('O[0-3]', metadata['optimization']):
            raise ValueError('unknown optimization profile')
        seen.add(metadata['optimization'])
        for field in ('revision', 'cpu', 'architecture', 'operating_system', 'affinity',
                      'source_sha256', 'compiler_sha256', 'clang_sha256', 'engine_sha256', 'engine'):
            if metadata[field] != documents[0]['metadata'][field]:
                raise ValueError('mixed compiler/input/host/engine identity: ' + field)
        if {row['id'] for row in document['configurations']} != {'scalar', 'simd128'}:
            raise ValueError('incomplete Wasm configuration set')
        for config in document['configurations']:
            if set(config['phases']) != {'compile', 'link', 'startup', *CASES}:
                raise ValueError('workload or build phase coverage is incomplete')
            expected = {(kind, count, seed) for kind in range(len(CASES)) for count in (1, 3)
                        for seed in (17, 42, 0x80000000, 0xffffffff)}
            if len(config['cases']) != 8 * len(CASES) or {tuple(row[:3]) for row in config['cases']} != expected:
                raise ValueError('independent oracle coverage is incomplete')
            for name, phase in config['phases'].items():
                for mode in ('AA', 'AB'):
                    required = {'seconds'} if name == 'startup' else {'seconds', 'cpu_seconds'}
                    if name in CASES:
                        required.add('setup_seconds')
                    if not required <= phase[mode].keys():
                        raise ValueError('missing timing metric: ' + name)
                    for metric in ('seconds', 'cpu_seconds', 'setup_seconds'):
                        if metric not in phase[mode]:
                            continue
                        values = phase[mode][metric]
                        if (not values['ratios'] or not all(math.isfinite(v) and v > 0 for v in values['ratios'])
                                or not math.isclose(values['speedup'], statistics.median(values['ratios']), rel_tol=1e-12)):
                            raise ValueError('invalid paired estimate')
    return documents


def normalized(documents, environment, run_url, details_url, raw_url):
    records = []
    validations = []
    for document in documents:
        metadata = document['metadata']
        identity = '|'.join(metadata[k] for k in ('cpu', 'architecture', 'operating_system')) + '|' + environment
        host = dict(id='wasm-' + hashlib.sha256(identity.encode()).hexdigest()[:16], cpu=metadata['cpu'],
                    architecture=metadata['architecture'], environment=environment)
        for config in document['configurations']:
            configuration = config['id']
            macro = '-DCPPGM_BENCH_SIMD=' + ('1' if configuration == 'simd128' else '0')
            flags = {name: ['--target=wasm32-unknown-unknown', *values, macro]
                     for name, values in metadata['flags'].items()}
            context = dict(language='C++11 with GNU vectors', target_isa='wasm32; SIMD128 permitted',
                           standard_library='none', workers=1, affinity=metadata['affinity'], flags=flags)
            base = dict(workload='wasm', configuration=configuration, host=host,
                        optimization=metadata['optimization'], revision=metadata['revision'], context=context,
                        details_url=details_url, raw_url=raw_url, run_url=run_url,
                        provenance=dict(metadata_sha256=hashlib.sha256(json.dumps(metadata, sort_keys=True).encode()).hexdigest(),
                                        warnings=document['warnings'], modules=config['module_sha256'],
                                        engine=metadata['engine'], oracle='Independent Perl unsigned arithmetic; '
                                        'all timed results also agree across compilers', raw_observations='CI artifacts'))
            def coverage(kind=None):
                cases = config['cases'] if kind is None else [r for r in config['cases'] if r[0] == kind]
                digest = hashlib.sha256(json.dumps(cases, separators=(',', ':')).encode()).hexdigest()
                return dict(selected=len(cases), candidate_executed=len(cases), candidate_passed=len(cases),
                            reference_executed=len(cases), reference_passed=len(cases), excluded=0,
                            unit='independent oracle cases', case_set_sha256=digest)
            def add(test, phase, metric, values, method, scope='summary', kind=None):
                cov = coverage(kind)
                row = dict(base, test=test, scope=scope, phase=phase, metric=metric, method=method,
                           baseline='clang', status='passed', coverage=cov, speedup=values['speedup'],
                           candidate_seconds=None, reference_seconds=None)
                suffix = 'value' if metric == 'bytes' else 'seconds'
                row['candidate_' + suffix] = values['candidate']
                row['reference_' + suffix] = values['reference']
                records.append(row)
                missing = copy.deepcopy(row)
                missing.update(baseline='gcc', status='not-applicable', speedup=None,
                               method='The selected GCC baseline does not emit the pinned Wasm target')
                missing['reference_' + suffix] = None
                missing['coverage'].update(reference_executed=0, reference_passed=0)
                records.append(missing)
            for phase in ('compile', 'link'):
                for metric, field in [('elapsed_seconds', 'seconds'), ('cpu_seconds', 'cpu_seconds')]:
                    add('one-translation-unit', phase, metric, config['phases'][phase]['AB'][field],
                        'Median paired ABBA block ratios; immutable input; includes tool process startup; '
                        'object compile and static link measured separately')
            for name in ('module', 'code', 'data'):
                sizes = config['module_sizes']
                a, b = sizes['cppgm'][name], sizes['clang'][name]
                add(name + '-size', 'link', 'bytes', dict(candidate=a, reference=b,
                    speedup=b / a if a > 0 else None),
                    'Final module bytes' if name == 'module' else 'Encoded ' + name + ' section payload bytes; excludes BSS',
                    scope='summary' if name == 'module' else 'test')
            add('instantiation-and-start', 'startup', 'elapsed_seconds', config['phases']['startup']['AB']['seconds'],
                'Median paired ABBA ratios; each observation averages 100 instantiations; '
                'excludes engine compilation, Store construction and destruction')
            for kind, name in enumerate(CASES):
                add(name, 'runtime', 'elapsed_seconds', config['phases'][name]['AB']['seconds'],
                    'Median paired ABBA block ratios for one warmed export call; '
                    'excludes engine compilation, instantiation and input preparation', scope='test', kind=kind)
                add(name, 'runtime', 'cpu_seconds', config['phases'][name]['AB']['cpu_seconds'],
                    'Median paired ABBA block ratios; process CPU clock around one warmed export call; '
                    'parallel engine compilation disabled; excludes input preparation', scope='test', kind=kind)
                add(name + '-input-preparation', 'startup', 'elapsed_seconds',
                    config['phases'][name]['AB']['setup_seconds'],
                    'Median paired ABBA block ratios for input preparation; excludes warmed workload execution',
                    scope='test', kind=kind)
            add('checked-workloads', 'runtime', 'elapsed_seconds', dict(candidate=None, reference=None,
                speedup=geometric(config['phases'][name]['AB']['seconds']['speedup'] for name in CASES)),
                'Equal-weight geometric mean of eight checked workload paired speedups; no cross-host aggregation')
            add('checked-workloads', 'runtime', 'cpu_seconds', dict(candidate=None, reference=None,
                speedup=geometric(config['phases'][name]['AB']['cpu_seconds']['speedup'] for name in CASES)),
                'Equal-weight geometric mean of eight checked workload paired CPU speedups; no cross-host aggregation')
            cov = coverage()
            validations.append(dict(base, oracle='Independent Perl arithmetic, seeded edge cases and cross-compiler checksums',
                status='passed', coverage={k: cov[k] for k in ('selected', 'excluded', 'unit', 'case_set_sha256')}))
            validations[-1]['coverage'].update(executed=cov['selected'], passed=cov['selected'])
    return dict(schema=1, records=records, validations=validations)


def report(documents, output, environment, run_url, details_url, raw_url):
    catalog = json.loads((ROOT / 'benchmarks/catalog.json').read_text())
    benchmark_report.render(catalog, normalized(documents, environment, run_url, details_url, raw_url), output)


def publish(args, documents):
    metadata = documents[0]['metadata']
    if any(d['metadata']['dirty'] for d in documents) or metadata['revision'] != args.revision:
        raise ValueError('publication requires the clean measured compiler revision')
    if {d['metadata']['optimization'] for d in documents} != {'O0', 'O3'}:
        raise ValueError('publication requires both O0 and O3')
    platform = benchmark_hosts.publication_platform(metadata['cpu'], args.platform)
    environment = benchmark_hosts.environment(platform)
    base = 'https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/wasm/'
    history = 'history/' + args.revision + '/' + platform
    host_metadata = dict(metadata, optimization='mixed',
                         flags={d['metadata']['optimization']: d['metadata']['flags'] for d in documents})
    with tempfile.TemporaryDirectory() as temporary:
        standard = Path(temporary) / 'standard'
        report(documents, standard, environment, args.run_url, base + history + '/report.md', args.run_url)
        for destination in (args.target / 'wasm' / platform, args.target / 'wasm' / history):
            destination.mkdir(parents=True, exist_ok=True)
            if (destination / 'standard').exists():
                shutil.rmtree(destination / 'standard')
            shutil.copytree(standard, destination / 'standard')
            (destination / 'metadata.json').write_text(json.dumps(host_metadata) + '\n')
            (destination / 'results.json').write_text(json.dumps(dict(schema=1, measurements=documents)) + '\n')
            (destination / 'report.md').write_text('# Wasm32 benchmark details\n\n'
                '[Standard comparison](standard/summary.md) · [Every kernel and metric](standard/wasm.md) · '
                '[Raw CI observations](' + args.run_url + ')\n')
    directory = args.target / 'wasm'
    benchmark_hosts.library_index(directory, 'Wasm32 benchmark hosts')
    seed(args.target)


def seed(target):
    directory = target / 'wasm'
    directory.mkdir(parents=True, exist_ok=True)
    for host, caption in HOSTS.items():
        path = directory / host / 'results.json'
        measured = json.loads(path.read_text())['measurements'] if path.exists() else []
        o3 = next((d for d in measured if d.get('schema') == 2 and d.get('status') == 'complete'
                   and d['metadata']['optimization'] == 'O3'), None)
        for configuration in ('scalar', 'simd128'):
            config = next((c for c in o3['configurations'] if c['id'] == configuration), None) if o3 else None
            for phase in ('compile', 'runtime'):
                value = None if config is None else (config['phases']['compile']['AB']['cpu_seconds']['speedup']
                    if phase == 'compile' else geometric(config['phases'][name]['AB']['cpu_seconds']['speedup'] for name in CASES))
                label = 'Wasm ' + configuration + ' ' + phase + ' vs Clang'
                if o3:
                    label += '; ' + o3['metadata']['cpu'] + '; revision ' + o3['metadata']['revision']
                (directory / f'{host}-{configuration}-{phase}-clang.svg').write_text(svg(label, caption, value))
    benchmark_hosts.library_index(directory, 'Wasm32 benchmark hosts')
    if not any((directory / host / 'metadata.json').exists() for host in HOSTS):
        (directory / 'report.md').write_text('# Wasm32 benchmark hosts\n\nNot measured yet. '
            '[Workloads and measurement method](https://github.com/vishvananda/cppgm-extended/blob/main/pa37/benchmarks/README.md).\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    for command in ('report', 'publish'):
        sub = commands.add_parser(command)
        sub.add_argument('artifacts', type=Path, nargs='+')
        sub.add_argument('--run-url', required=True)
        if command == 'report':
            sub.add_argument('--output', type=Path, required=True)
            sub.add_argument('--environment', required=True)
        else:
            sub.add_argument('--target', type=Path, required=True)
            sub.add_argument('--revision', required=True)
            sub.add_argument('--platform', choices=['github', 'intel'], default='github')
    args = parser.parse_args()
    documents = load(args.artifacts)
    if args.command == 'publish':
        publish(args, documents)
    else:
        report(documents, args.output, args.environment, args.run_url, args.run_url, args.run_url)


if __name__ == '__main__':
    main()

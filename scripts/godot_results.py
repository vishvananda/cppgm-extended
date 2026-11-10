#!/usr/bin/env python3
"""Adapt validated Godot measurements to the shared benchmark record format."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import statistics

import benchmark_report
import godot_ci as producer


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def normalize(artifact, environment, run_url, details_url, raw_url):
    document = json.loads((artifact / 'results.json').read_text())
    producer.validate(document)
    metadata = document['metadata']
    if metadata['dirty']:
        raise ValueError('shared publication requires a clean measured revision')
    host = dict(cpu=metadata['cpu'], architecture='x86_64', environment=environment)
    host['id'] = 'godot-' + digest(host)[:16]
    observations = [json.loads(line) for line in (artifact / 'runtime-observations.jsonl').read_text().splitlines()]
    provenance = dict(source_commit=document['source_commit'], metadata=metadata,
                      independent_checksums=document['correctness'],
                      binary_hashes=document.get('binary_hashes', 'not recorded'),
                      runtime_observations_sha256=hashlib.sha256(
                          (artifact / 'runtime-observations.jsonl').read_bytes()).hexdigest(),
                      command_manifests={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                                         for p in sorted(artifact.glob('commands-*.json'))},
                      historical_unknowns={key: 'not recorded' for key in
                                           ('system', 'standard_library') if key not in metadata})
    identity = dict(source=producer.COMMIT, configuration='headless-release',
                    workload_hashes=metadata['workload_hashes'], iterations=100000)
    flags = {label: dict(scons=metadata['flags'], command_manifest='commands-' + label + '.json',
                        additional_cpp_flags=[])
             for label in producer.LABELS}
    links = dict(run_url=run_url, details_url=details_url, raw_url=raw_url)
    base = dict(workload='godot', configuration='headless-release', optimization='O3',
                host=host, revision=metadata['revision'], status='passed', provenance=provenance, **links)

    def coverage(cases, unit):
        count = len(cases)
        return dict(selected=count, candidate_executed=count, candidate_passed=count,
                    reference_executed=count, reference_passed=count, excluded=0, unit=unit,
                    case_set_sha256=digest(dict(identity, cases=list(cases))))

    def context(phase):
        cpus = [int(c) for c in metadata['cpus'].split(',')]
        affinity = cpus if phase == 'build' else [int(c) for c in metadata['runtime_cpu'].split(',')]
        jobs = metadata.get('jobs')
        if jobs is None:
            jobs = int(next(arg[2:] for arg in document['compilation']['cppgm']['command']
                            if arg.startswith('-j') and arg[2:].isdigit()))
        return dict(language='GNU C++17; C sources and final link use GCC',
                    target_isa='x86-64, SSE4.2, POPCNT; no native CPU tuning',
                    standard_library=metadata.get('standard_library', 'host libstdc++; exact identity not recorded'),
                    workers=jobs if phase == 'build' else 1,
                    affinity=affinity, flags=flags)

    records = []
    for ref in ('gcc', 'clang'):
        for metric, build_key, runtime_key in [('cpu_seconds', 'cpu', 'cpu'),
                                               ('elapsed_seconds', 'wall', 'measured_wall')]:
            candidate = document['compilation']['cppgm'][build_key]
            reference = document['compilation'][ref][build_key]
            records.append(dict(base, baseline=ref, test='full-build', scope='summary', phase='build',
                                context=context('build'), metric=metric,
                                method='One clean full build per compiler; includes SCons, C compilation and GCC link',
                                coverage=coverage(['full-build'], 'full builds'), speedup=reference / candidate,
                                provenance=dict(provenance, build_observations={
                                    label: document['compilation'][label] for label in ('cppgm', ref)}),
                                candidate_seconds=candidate, reference_seconds=reference))
            for name in producer.WORKLOADS:
                rows = [r for r in observations if r.get('phase') == 'AB-' + ref and r.get('workload') == name]
                def absolute(label):
                    values = [r[runtime_key] for r in rows if r['label'] == label]
                    if not values:
                        raise ValueError('missing raw runtime observations: ' + name + ' / ' + label)
                    return statistics.median(values)
                records.append(dict(base, baseline=ref, test=name, scope='test', phase='runtime',
                                    context=context('runtime'), metric=metric,
                                    method='Median paired ABBA/BAAB block ratios; absolute times are observation medians; '
                                           + ('process CPU includes startup' if runtime_key == 'cpu' else 'work elapsed excludes startup'),
                                    coverage=coverage([name], 'checked workloads'),
                                    provenance=dict(provenance, timing=document['runtime'][name][ref][runtime_key]),
                                    speedup=document['runtime'][name][ref][runtime_key]['speed'],
                                    candidate_seconds=absolute('cppgm'), reference_seconds=absolute(ref)))
            value = math.exp(statistics.mean(math.log(document['runtime'][name][ref][runtime_key]['speed'])
                                             for name in producer.WORKLOADS))
            records.append(dict(base, baseline=ref, test='checked-workloads', scope='summary', phase='runtime',
                                context=context('runtime'), metric=metric,
                                method='Equal-weight geometric mean of all five median paired block speedups; '
                                       + ('process CPU includes startup' if runtime_key == 'cpu' else 'work elapsed excludes startup'),
                                coverage=coverage(producer.WORKLOADS, 'checked workloads'), speedup=value,
                                provenance=dict(provenance, timing={
                                    name: document['runtime'][name][ref][runtime_key] for name in producer.WORKLOADS}),
                                candidate_seconds=None, reference_seconds=None))
    cov = coverage(producer.WORKLOADS, 'checked workloads per compiler')
    validation = dict(base, oracle='Independent Python checksums for cppgm, GCC and Clang; full builds succeed',
                      coverage={key: cov[key] for key in ('selected', 'excluded', 'unit', 'case_set_sha256')})
    validation['coverage'].update(executed=cov['selected'], passed=cov['selected'])
    return dict(schema=1, records=records, validations=[validation])


def generate(artifact, output, environment, run_url, details_url, raw_url):
    catalog = json.loads((producer.ROOT / 'benchmarks/catalog.json').read_text())
    document = normalize(artifact, environment, run_url, details_url, raw_url)
    benchmark_report.render(catalog, document, output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('artifact', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--environment', required=True)
    for key in ('run-url', 'details-url', 'raw-url'):
        parser.add_argument('--' + key, required=True)
    args = parser.parse_args()
    generate(args.artifact, args.output, args.environment, args.run_url, args.details_url, args.raw_url)


if __name__ == '__main__':
    main()

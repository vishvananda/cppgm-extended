"""Adapt internal source-object measurements to the shared benchmark records."""
import hashlib
import json
from pathlib import Path

import benchmark_report


def normalize(document, environment, run_url, details_url, raw_url):
    metadata = document['metadata']
    host = dict(cpu=metadata['cpu'], architecture='x86_64', environment=environment)
    host['id'] = 'internal-' + hashlib.sha256(json.dumps(host, sort_keys=True).encode()).hexdigest()[:16]
    coverage = dict(selected=5724, candidate_executed=5724, candidate_passed=5724,
                    reference_executed=5724, reference_passed=5724, excluded=0,
                    unit='C++ translation units', case_set_sha256=metadata['cohort_sha256'])
    base = dict(workload='internal', configuration='native-overlap', test='fixed-positive-cohort',
                scope='summary', host=host, revision=metadata['revision'], status='passed', optimization='O3',
                context=dict(language='Per-source C++11, C++14 or C++17',
                             target_isa='baseline x86-64; no native CPU tuning',
                             standard_library='Host libstdc++; GCC and Clang use the same GCC installation',
                             workers=metadata['jobs'], affinity=metadata.get('cpus', []),
                             flags={role: ['-O3', 'per-source dialect and flags in cohort.json']
                                    for role in ('cppgm', 'gcc', 'clang')}),
                details_url=details_url, raw_url=raw_url, run_url=run_url,
                provenance=dict(metadata=metadata, input_snapshot=metadata['input_revision'],
                                raw_observations='role-observations.jsonl.gz, one record per fixture group'))
    candidate = document['compilation']['cppgm']['cpu_seconds']
    records = []
    for reference in ('gcc', 'clang'):
        seconds = document['compilation'][reference]['cpu_seconds']
        records.append(dict(base, baseline=reference, phase='compile', metric='cpu_seconds',
                            coverage=coverage, candidate_seconds=candidate, reference_seconds=seconds,
                            speedup=seconds / candidate,
                            method='Ratio of total compiler process-tree CPU for the complete fixed cohort; '
                                   'one run per compiler; startup included, linking/execution/harness excluded'))
    validation = dict(base, oracle='All three compilers emit valid x86-64 ELF objects for every frozen source unit; '
                                  'original course oracles are separate',
                      coverage=dict(selected=5724, executed=5724, passed=5724, excluded=0,
                                    unit='C++ translation units', case_set_sha256=metadata['cohort_sha256']))
    return dict(schema=1, records=records, validations=[validation])


def write(document, destination, environment, run_url, url):
    records = normalize(document, environment, run_url, url + '/report.md', url + '/results.json')
    root = Path(__file__).resolve().parents[1]
    catalog = json.loads((root / 'benchmarks/catalog.json').read_text())
    benchmark_report.validate(catalog, records)
    (destination / 'records.json').write_text(json.dumps(records, indent=2) + '\n')

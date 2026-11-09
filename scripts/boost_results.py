#!/usr/bin/env python3
"""Adapt the reviewed Boost campaign to shared host-specific comparisons."""
import hashlib
import json
from pathlib import Path
import re

import benchmark_report
import boost_benchmarks as producer
import boost_ci


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()


def normalize(artifact, environment, run_url, details_url, raw_url):
    document = json.loads((artifact / 'results.json').read_text())
    producer.validate(document)
    metadata = document['metadata']
    host = dict(cpu=metadata['cpu'], architecture='x86_64', environment=environment)
    host['id'] = 'boost-' + digest(host)[:16]
    suites = {label: {s['project']: s['tests'] for s in report['suites']}
              for label, report in document['compilers'].items()}
    flags = {label: dict(b2=metadata['flags'], invocations=label + '/libs-*/invocations.jsonl')
             for label in producer.LABELS}
    provenance = dict(archive_sha256=document['archive_sha256'], metadata=metadata,
                      raw_per_test_timings='results.json compilers/label/suites/tests',
                      source='fresh extraction of the verified upstream archive')
    base = dict(workload='boost', configuration='o3' if producer.optimized(document) else 'debug', host=host,
                revision=metadata['revision'], status='passed', provenance=provenance,
                context=dict(language='C++11', target_isa='baseline x86-64; no native CPU tuning',
                             standard_library=metadata['standard_library'], workers=metadata['jobs'],
                             affinity=metadata['cpus'], flags=flags),
                run_url=run_url, details_url=details_url, raw_url=raw_url)

    def coverage(rows, unit):
        count = len(rows)
        return dict(selected=count, candidate_executed=count, candidate_passed=count,
                    reference_executed=count, reference_passed=count, excluded=0, unit=unit,
                    case_set_sha256=digest(dict(archive=document['archive_sha256'],
                                               adaptations=metadata.get('source_adaptations', []),
                                               cases=sorted((r['target'], r['kind']) for r in rows))))

    records = []
    for project in [*boost_ci.SUITES, 'all-suites']:
        rows = {label: (list(producer.cases(document['compilers'][label]).values()) if project == 'all-suites'
                        else suites[label][project]) for label in producer.LABELS}
        scope = 'summary' if project == 'all-suites' else 'test'
        levels = set()
        for label in producer.LABELS:
            for row in rows[label]:
                for command in row.get('sources', []):
                    selected = [flag[1:] for flag in command if re.fullmatch(r'-O[0-3sgz]', flag)]
                    levels.add(selected[-1] if selected else 'O0')
        optimization = next(iter(levels)) if len(levels) == 1 and not levels.intersection({'Og'}) else 'mixed'
        for phase, key, unit, method in [
                ('compile', 'compile_cpu', 'test declarations',
                 'Ratio of sums of C++ compilation child CPU seconds for the identical reviewed test declarations; '
                 'one clean campaign per compiler; configuration probes, library dependencies, C, linking and B2 excluded'),
                ('test-execution', 'test_cpu', 'native test actions',
                 'Ratio of sums of native test child CPU seconds, including startup and GDB integration; '
                 'one clean campaign per compiler; correctness-test timing, not application throughput')]:
            selected = {label: [r for r in value if (phase != 'compile' or not producer.optimized(document) or r['target'] != producer.boost_runtime.GDB)
                               and (phase == 'compile' or r['kind'] in ('RUN', 'RUN_FAIL'))]
                        for label, value in rows.items()}
            if not selected['cppgm']:
                continue
            candidate = sum(r.get(key, 0) for r in selected['cppgm'])
            for ref in ('gcc', 'clang'):
                reference = sum(r.get(key, 0) for r in selected[ref])
                if min(candidate, reference) <= 0:
                    raise ValueError('missing positive Boost timing')
                records.append(dict(base, baseline=ref, test=project, scope=scope, phase=phase, optimization='O3' if phase == 'compile' and producer.optimized(document) else optimization,
                                    metric='cpu_seconds', method=method, coverage=coverage(selected['cppgm'], unit),
                                    speedup=reference / candidate,
                                    candidate_seconds=candidate, reference_seconds=reference))
    runtime = document.get('runtime')
    if runtime is not None:
        for project, group in [('all-suites', runtime), *runtime['suites'].items()]:
            selected = [r for r in producer.cases(document['compilers']['cppgm']).values()
                        if producer.boost_runtime.eligible(r) and (project == 'all-suites' or r['target'].split('//')[0] == project)]
            cov = coverage(selected, 'native test programs')
            cov['excluded'] = sum(project == 'all-suites' or r['target'].split('//')[0] == project
                                  for r in runtime['exclusions'])
            for reference, value in group['comparisons'].items():
                records.append(dict(base, baseline=reference, test=project, scope='summary' if project == 'all-suites' else 'test',
                                    phase='runtime', optimization='O3' if producer.optimized(document) else 'O0',
                                    metric='cpu_seconds', coverage=cov, speedup=value['speedup'],
                                    candidate_seconds=value['candidate_seconds'], reference_seconds=value['reference_seconds'],
                                    method='Median reference/cppgm speedup of mirrored ABBA/BAAB block CPU means; '
                                    'two warmups per compiler, four A/A observations per reference; repeated short programs normalized '
                                    'to one execution; verification and GDB excluded; native process startup included',
                                    provenance=dict(provenance, runtime_replay={k: runtime[k] for k in
                                                    ('selected', 'blocks', 'output_equality_cases', 'case_set_sha256')},
                                                    paired_blocks=value['blocks'], calibration=value['calibration'],
                                                    raw_runtime_observations='runtime/batches.jsonl and runtime/NNN-*.jsonl')))
    all_rows = list(producer.cases(document['compilers']['cppgm']).values())
    cov = coverage(all_rows, 'Boost test declarations per compiler')
    validation = dict(base, optimization='mixed', oracle='Original B2 compile/link/run and expected-failure actions, including GDB; all three compilers',
                      coverage={key: cov[key] for key in ('selected', 'excluded', 'unit', 'case_set_sha256')})
    validation['coverage'].update(executed=len(all_rows), passed=len(all_rows))
    return dict(schema=1, records=records, validations=[validation])


def generate(artifact, output, environment, run_url, details_url, raw_url):
    catalog = json.loads((producer.ROOT / 'benchmarks/catalog.json').read_text())
    benchmark_report.render(catalog, normalize(artifact, environment, run_url, details_url, raw_url), output)

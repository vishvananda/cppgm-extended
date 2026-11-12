#!/usr/bin/env python3
"""Validate workload-neutral comparison records and render linked result tables."""
import argparse
from collections import defaultdict
import json
import math
from pathlib import Path
import re
from urllib.parse import urlparse

PHASES = {'compile', 'link', 'build', 'startup', 'runtime', 'test-execution', 'suite'}
STATUSES = {'passed', 'failed', 'partial', 'not-measured', 'not-applicable'}
LEVELS = {'O0', 'O1', 'O2', 'O3', 'Os', 'Oz', 'mixed'}
IDENTIFIER = re.compile(r'[a-z0-9][a-z0-9_-]*\Z')


def require(condition, message):
    if not condition:
        raise ValueError(message)


def positive(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool) and math.isfinite(value) and value > 0


def catalog_entries(catalog):
    require(catalog.get('schema') == 1, 'Unknown catalog schema')
    entries = {}
    for workload in catalog['workloads']:
        key = workload['id']
        require(IDENTIFIER.fullmatch(key) and key not in entries, 'Invalid/duplicate workload')
        configs = workload['configurations']
        require(configs or workload.get('status') == 'planned', 'Active workload needs configurations')
        ids = [c['id'] for c in configs]
        require(len(set(ids)) == len(ids) and all(IDENTIFIER.fullmatch(x) for x in ids),
                'Invalid/duplicate configuration')
        entries[key] = workload
    return entries


def validate_correctness(entries, rows, hosts):
    seen = set()
    for row in rows:
        entry = entries.get(row['workload'])
        require(entry is not None and row['configuration'] in {c['id'] for c in entry['configurations']},
                'Unknown correctness workload/configuration')
        require(re.fullmatch(r'[0-9a-f]{40}', row['revision']), 'Measured revision must be a full SHA')
        require(row['optimization'] in LEVELS and bool(row['oracle']), 'Correctness level/oracle required')
        host = row['host']
        require(all(isinstance(host.get(k), str) and host[k] for k in
                    ['id', 'cpu', 'architecture', 'environment']), 'Incomplete host identity')
        identity = tuple(host[k] for k in ['cpu', 'architecture', 'environment'])
        require(hosts.setdefault(host['id'], identity) == identity, 'Host ID merges CPU models/environments')
        coverage = row['coverage']
        require(all(type(coverage[k]) is int and coverage[k] >= 0 for k in
                    ['selected', 'executed', 'passed', 'excluded']), 'Invalid correctness coverage count')
        require(coverage['passed'] <= coverage['executed'] <= coverage['selected'], 'Impossible coverage counts')
        require(bool(coverage['unit']) and re.fullmatch(r'[0-9a-f]{64}', coverage['case_set_sha256']),
                'Missing case-set identity/unit')
        require(row['status'] in {'passed', 'failed', 'partial'}, 'Invalid correctness status')
        complete = coverage['selected'] > 0 and coverage['passed'] == coverage['executed'] == coverage['selected']
        require(row['status'] != 'passed' or complete, 'Passing correctness requires complete validation')
        for k in ['details_url', 'raw_url', 'run_url']:
            link = urlparse(row[k])
            require(link.scheme == 'https' and bool(link.netloc), 'Evidence must have HTTPS URLs')
        require(isinstance(row['provenance'], dict), 'Producer provenance is required')
        key = tuple(row[k] for k in ['workload', 'configuration', 'revision', 'optimization', 'oracle'])
        key += (host['id'], coverage['case_set_sha256'])
        require(key not in seen, 'Duplicate correctness identity')
        seen.add(key)


def validate(catalog, document):
    entries = catalog_entries(catalog)
    require(document.get('schema') == 1, 'Unknown results schema')
    seen, hosts = set(), {}
    validate_correctness(entries, document.get('validations', []), hosts)
    for row in document['records']:
        workload = entries.get(row['workload'])
        require(workload is not None, 'Unknown workload')
        require(row['configuration'] in {c['id'] for c in workload['configurations']}, 'Unknown configuration')
        require(row['baseline'] in {'gcc', 'clang'}, 'Baseline must name GCC or Clang')
        require(row['optimization'] in LEVELS, 'Missing/unknown optimization level')
        require(row['phase'] in PHASES, 'Unknown measurement phase')
        require(row['scope'] in {'summary', 'test'}, 'Summary/test scope must be explicit')
        require(row['metric'] in {'cpu_seconds', 'elapsed_seconds', 'bytes'}, 'Measurement unit must be explicit')
        require(row['status'] in STATUSES, 'Unknown result status')
        require(re.fullmatch(r'[0-9a-f]{40}', row['revision']), 'Measured revision must be a full SHA')
        require(bool(row['test']) and bool(row['method']), 'Test and estimator are required')
        host = row['host']
        require(all(host.get(k) for k in ['id', 'cpu', 'architecture', 'environment']), 'Incomplete host identity')
        require(all(isinstance(host[k], str) for k in ['id', 'cpu', 'architecture', 'environment']), 'Host fields must be strings')
        context = row['context']
        require(all(isinstance(context.get(k), str) and context[k] for k in
                    ['language', 'target_isa', 'standard_library']), 'Language, ISA and library identity required')
        require(type(context.get('workers')) is int and context['workers'] > 0, 'Worker count required')
        require(isinstance(context.get('affinity'), list) and isinstance(context.get('flags'), dict),
                'Affinity and per-compiler flags required')
        identity = tuple(host[k] for k in ['cpu', 'architecture', 'environment'])
        require(hosts.setdefault(host['id'], identity) == identity, 'Host ID merges CPU models/environments')
        key = tuple(row[k] for k in ['workload', 'configuration', 'test', 'scope', 'optimization', 'baseline', 'phase', 'metric', 'revision']) + (host['id'], json.dumps(context, sort_keys=True))
        require(key not in seen, 'Duplicate comparison identity')
        seen.add(key)
        coverage = row['coverage']
        for k in ['selected', 'candidate_executed', 'candidate_passed', 'reference_executed', 'reference_passed', 'excluded']:
            require(type(coverage[k]) is int and coverage[k] >= 0, 'Invalid coverage count')
        for side in ['candidate', 'reference']:
            require(coverage[side+'_passed'] <= coverage[side+'_executed'] <= coverage['selected'], 'Impossible coverage counts')
        require(bool(coverage['unit']) and re.fullmatch(r'[0-9a-f]{64}', coverage['case_set_sha256']), 'Missing case-set identity/unit')
        complete = coverage['selected'] > 0 and all(coverage[k] == coverage['selected'] for k in
                   ['candidate_executed', 'candidate_passed', 'reference_executed', 'reference_passed'])
        require(row['status'] != 'passed' or complete, 'Passing result requires complete matching validation')
        speed = row.get('speedup')
        zero_size_ratio = row['metric'] == 'bytes' and type(speed) in (int, float) and speed == 0
        require(speed is None or ((positive(speed) or zero_size_ratio) and row['status'] == 'passed' and complete), 'Ineligible speedup')
        for k in ['candidate_seconds', 'reference_seconds']:
            require(row.get(k) is None or positive(row[k]), 'Absolute times must be positive or absent')
            require(row['metric'] != 'bytes' or row.get(k) is None, 'Bytes cannot use seconds fields')
        for k in ['candidate_value', 'reference_value']:
            require(row.get(k) is None or (row['metric'] == 'bytes' and type(row[k]) is int and row[k] >= 0),
                    'Byte values must be nonnegative integers')
            require(row['metric'] != 'bytes' or row['status'] != 'passed' or row.get(k) is not None,
                    'Passing byte measurements require absolute values')
        if row['metric'] == 'bytes' and row['status'] == 'passed':
            candidate, reference = row['candidate_value'], row['reference_value']
            require(speed == reference / candidate if candidate else speed is None,
                    'Byte ratio must match the measured sizes; zero denominator has no ratio')
        for k in ['details_url', 'raw_url', 'run_url']:
            link = urlparse(row[k])
            require(link.scheme == 'https' and bool(link.netloc), 'Evidence must have HTTPS URLs')
        require(isinstance(row['provenance'], dict), 'Producer provenance is required')
    return entries


def cell(value):
    return str(value).replace('|', '\\|').replace('\r', ' ').replace('\n', ' ')


def speed(row):
    return f"{row['speedup']:.3f}×" if row.get('speedup') is not None else (
        'n/a' if row['status'] == 'passed' else row['status'])


def measurement(row, side):
    if row['metric'] == 'bytes':
        value = row.get(side + '_value')
        return str(value) if value is not None else 'not provided'
    value = row.get(side + '_seconds')
    return f'{value:.6f}' if value is not None else 'not provided'


def pairing_context(row):
    context = dict(row['context'])
    # Native flags may differ between baselines; candidate build flags may not.
    context['flags'] = {'cppgm': context['flags'].get('cppgm')}
    return json.dumps(context, sort_keys=True)


def missing_baselines(rows):
    """Presentation placeholders do not become fabricated measurement records."""
    groups = defaultdict(dict)
    for row in rows:
        key = tuple(row[k] for k in ['configuration', 'test', 'optimization', 'phase', 'metric', 'revision'])
        key += (row['coverage']['case_set_sha256'], pairing_context(row))
        groups[key][row['baseline']] = row
    result = list(rows)
    for comparisons in groups.values():
        for baseline in {'gcc', 'clang'} - comparisons.keys():
            row = dict(next(iter(comparisons.values())), baseline=baseline, status='not-measured',
                       speedup=None, candidate_seconds=None, reference_seconds=None)
            row['coverage'] = dict(row['coverage'], candidate_passed=0, reference_passed=0)
            result.append(row)
    return result


def render(catalog, document, output):
    entries = validate(catalog, document)
    output = Path(output)
    output.mkdir(parents=True, exist_ok=False)
    (output/'records.json').write_text(json.dumps(document, indent=2)+'\n')
    (output/'catalog.json').write_text(json.dumps(catalog, indent=2)+'\n')
    by_host, by_workload, by_validation = defaultdict(list), defaultdict(list), defaultdict(list)
    for row in document.get('validations', []):
        by_validation[row['workload']].append(row)
    for row in document['records']:
        if row['scope'] == 'summary':
            by_host[row['host']['id']].append(row)
        by_workload[row['workload']].append(row)
    lines = ['# Correctness and performance', '',
             'Speedup = reference time / cppgm time; higher is better. CPU and elapsed metrics are separate.',
             'Each row keeps GCC and Clang separate for one phase and metric. Missing values are not passes or zeros.', '',
             'Availability describes the supplied records; measurements from other reports may not have been imported.', '',
             '| Workload / configuration | Optimization levels | Availability | Details |', '|---|---|---|---|']
    for key, entry in entries.items():
        for config in entry['configurations'] or [dict(id='', name='configuration not yet declared', optimizations=[])]:
            present = [r for r in by_workload.get(key, []) if r['configuration'] == config['id']]
            validated = [r for r in by_validation.get(key, []) if r['configuration'] == config['id']]
            available = ('recorded' if present else 'correctness recorded; timing not measured' if validated
                         else 'planned' if entry.get('status') == 'planned' else 'not measured')
            name = cell(entry['name']+' / '+config['name'])
            link = f'[{name}]({key}.md)' if present or validated else name
            levels = ', '.join(config.get('optimizations', [])) or 'not yet declared'
            measured = ', '.join(sorted({r['optimization'] for r in present})) or 'none'
            lines.append(f'| {link} | declared: {levels}; recorded: {measured} | {available} | {cell(entry.get("documentation", "not yet documented"))} |')
    if document.get('validations'):
        lines += ['', '## Correctness coverage', '',
                  '| Workload / configuration | Oracle | CPU / environment | Opt | Passed / executed / selected | Excluded | Status | Revision |',
                  '|---|---|---|---|---|---|---|---|']
        for row in document['validations']:
            cov = row['coverage']; label = row['workload']+'/'+row['configuration']
            values = [f'[{label}]({row["workload"]}.md)', row['oracle'],
                      row['host']['cpu']+' / '+row['host']['environment'], row['optimization'],
                      f"{cov['passed']} / {cov['executed']} / {cov['selected']} {cov['unit']}",
                      cov['excluded'], row['status'], row['revision'][:9]]
            lines.append('| '+' | '.join(cell(x) for x in values)+' |')
    for host_id, rows in sorted(by_host.items()):
        host = rows[0]['host']
        lines += ['', f'## {cell(host["cpu"])} — {cell(host["environment"])}', '',
                  f'Architecture: {cell(host["architecture"])}; host profile `{cell(host_id)}`.', '',
                  '| Workload / configuration / test | Opt | Phase / metric | vs GCC | vs Clang | Validation | Case set | Revision |',
                  '|---|---|---|---:|---:|---|---|---|']
        # A workload name is part of the key: distinct suites may share test IDs.
        complete_rows = []
        for workload in {r['workload'] for r in rows}:
            complete_rows += missing_baselines([r for r in rows if r['workload'] == workload])
        paired = defaultdict(dict)
        for row in complete_rows:
            key = tuple(row[k] for k in ['workload', 'configuration', 'test', 'optimization', 'phase', 'metric', 'revision'])
            key += (row['coverage']['case_set_sha256'], pairing_context(row))
            paired[key][row['baseline']] = row
        for _, comparisons in sorted(paired.items()):
            row = next(iter(comparisons.values()))
            cov = row['coverage']; label = '/'.join(row[k] for k in ['workload', 'configuration', 'test'])
            cases = f"{cov['selected']} {cov['unit']}; set {cov['case_set_sha256'][:8]}"
            validation = '; '.join(f"{name}: {r['coverage']['candidate_passed']}/{r['coverage']['selected']} cppgm, "
                                   f"{r['coverage']['reference_passed']}/{r['coverage']['selected']} reference"
                                   if r['status'] == 'passed' else f"{name}: {r['status']}"
                                   for name, r in sorted(comparisons.items()))
            lines.append(f'| [{cell(label)}]({row["workload"]}.md) | {row["optimization"]} | {row["phase"]} / {row["metric"]} | {speed(comparisons["gcc"])} | {speed(comparisons["clang"])} | {cell(validation)} | {cell(cases)} | `{row["revision"][:9]}` |')
    (output/'summary.md').write_text('\n'.join(lines)+'\n')
    for workload in sorted(by_workload.keys() | by_validation.keys()):
        rows = by_workload[workload]
        detail = ['# '+entries[workload]['name'], '',
                  '| Configuration / test | CPU / environment | Opt | Baseline | Phase / metric | cppgm value | Reference value | Speedup | Validation | Revision | Method | Evidence |',
                  '|---|---|---|---|---|---:|---:|---:|---|---|---|---|']
        for row in rows:
            detail.append('| '+ ' | '.join(cell(x) for x in [row['configuration']+'/'+row['test'],
                row['host']['cpu']+' / '+row['host']['environment'], row['optimization'], row['baseline'],
                row['phase']+' / '+row['metric'], measurement(row, 'candidate'), measurement(row, 'reference'),
                speed(row), f"{row['coverage']['candidate_passed']} candidate / {row['coverage']['reference_passed']} reference / {row['coverage']['selected']} selected; {row['coverage']['excluded']} excluded", row['revision'][:9],
                row['method'], f'[details]({row["details_url"]}) · [raw]({row["raw_url"]}) · [CI]({row["run_url"]})'])+' |')
        for row in by_validation[workload]:
            cov = row['coverage']
            detail += ['', f"Correctness: {cell(row['configuration'])}, {cell(row['oracle'])}, {cell(row['status'])}; "
                       f"{cov['passed']}/{cov['executed']}/{cov['selected']} passed/executed/selected {cell(cov['unit'])}; "
                       f"{cov['excluded']} excluded; set `{cov['case_set_sha256']}`; revision `{row['revision']}`.",
                       f'[details]({row["details_url"]}) · [raw]({row["raw_url"]}) · [CI]({row["run_url"]})']
        warnings = sorted({cell(warning) for row in rows for warning in row['provenance'].get('warnings', [])})
        if warnings:
            detail += ['', 'Measurement cautions:', ''] + ['- ' + warning for warning in warnings]
        detail += ['', 'The retained records.json contains coverage, case-set hashes and producer provenance for every row.']
        (output/(workload+'.md')).write_text('\n'.join(detail)+'\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--catalog', type=Path, required=True)
    parser.add_argument('--results', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    render(json.loads(args.catalog.read_text()), json.loads(args.results.read_text()), args.output)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Verify saved GCC campaign evidence and screen language outcomes.

This does not certify conformance. Negative inputs still need standards
adjudication, isolated regressions and valid neighboring controls. Native
diagnostic patterns remain annotations and never enter the score.
"""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path

from summarize_upstream_dg_campaign import summarize
from upstream_dg_proxy import sha256, write_json
from upstream_language_outcomes import classify


def project(root, dispositions):
    verified = summarize(root)
    if not verified['snapshot_complete']:
        raise ValueError('Incomplete campaign')
    if set(verified['profile']['modes']) != {'host', 'candidate'}:
        raise ValueError('Language screening requires paired host/candidate evidence')
    rows = []
    for pair in verified['pairs']:
        states, data = {}, {}
        for mode in ['host', 'candidate']:
            row = pair.get(mode)
            if not row:
                raise ValueError('Missing pair member')
            states[mode] = json.loads(Path(row['state']).read_text())
            data[mode] = json.loads(Path(row['result']).read_text()) if row['result'] else dict(status=row['status'])
        host_task, candidate_task = [states[mode]['task'] for mode in ['host', 'candidate']]
        if any(host_task[key] != candidate_task[key] for key in ['source', 'sha256', 'revision']):
            raise ValueError('Paired source identity mismatch')
        disposition = dispositions.get(pair['identity'])
        if disposition and disposition['sha256'] != host_task['sha256']:
            raise ValueError('Disposition does not match frozen source')
        result = classify(data['host'], data['candidate'], host_task['source'], disposition)
        rows.append(dict(identity=pair['identity'], dialect=pair['dialect'],
                         source_sha256=host_task['sha256'], **result,
                         host_result=pair['host']['result'], candidate_result=pair['candidate']['result'],
                         native_host_status=pair['host']['status'], native_candidate_status=pair['candidate']['status']))
    counts = Counter(row['status'] for row in rows)
    screened = sum(counts[status] for status in ['outcome-match', 'outcome-mismatch', 'runtime-mismatch', 'compiler-abnormal'])
    by_expected = defaultdict(Counter)
    for row in rows:
        by_expected[row['expected'] or 'unresolved'][row['status']] += 1
    return dict(campaign=str(root), candidate_sha256=sha256(root / 'candidate'),
                profile=verified['profile'], verified_terminal_cells=verified['verified_terminal_cells'],
                verified_raw_streams=verified['verified_raw_streams'], verified_outputs=verified['verified_outputs'],
                selected_variants=len(rows), selected_sources=len({row['identity'] for row in rows}),
                counts=dict(counts), by_expected={key: dict(value) for key, value in by_expected.items()},
                screened_variants=screened, provisional_outcome_match_percent=100 * counts['outcome-match'] / screened if screened else None,
                diagnostic_parity_required=False, full_corpus_correctness_percent=None,
                interpretation='Selected problem-cohort outcome screen; mismatches need standards triage and negative matches need rule isolation. Blocked/review/not-required cases are separate, never passes.',
                rows=rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--campaign', type=Path, required=True)
    parser.add_argument('--dispositions', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    dispositions = json.loads(args.dispositions.read_text())
    result = project(args.campaign.resolve(), dispositions)
    result['projection_scripts'] = {path.name: sha256(path) for path in [
        Path(__file__), Path(__file__).with_name('upstream_language_outcomes.py'),
        Path(__file__).with_name('summarize_upstream_dg_campaign.py')]}
    result['dispositions_sha256'] = sha256(args.dispositions)
    args.output.mkdir(parents=True, exist_ok=False)
    write_json(args.output / 'results.json', result)
    write_json(args.output / 'dispositions.json', dispositions)
    print(json.dumps({key: value for key, value in result.items() if key != 'rows'}, indent=2))


if __name__ == '__main__':
    main()

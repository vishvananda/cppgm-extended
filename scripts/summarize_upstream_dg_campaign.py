#!/usr/bin/env python3
"""Verify native-dg campaign evidence and pair host/candidate outcomes.

Counts describe native oracle outcomes for declared variants. They do not
adjudicate C++11 requirements, infer root causes or establish conformance.
"""
import argparse
from collections import Counter, defaultdict
import csv
import json
from pathlib import Path
import sys

from upstream_dg_proxy import sha256, write_json


def pair_status(host, candidate):
    if not host or not candidate:
        return 'incomplete-pair'
    if host != 'native-assertions-passed':
        return 'host-baseline-not-passing'
    if candidate == 'native-assertions-passed':
        return 'native-oracles-satisfied'
    if candidate.startswith('harness-') or candidate == 'probe-abnormal':
        return 'candidate-adapter-or-probe-blocked'
    if candidate in {'compiler-abnormal', 'runtime-abnormal'}:
        return 'candidate-abnormal'
    if candidate == 'native-failed':
        return 'candidate-native-oracle-failure'
    return 'candidate-capability-or-variant-unresolved'


def failed_assertions(data):
    counts = Counter()
    for item in data.get('assertions', []):
        if item['status'] not in {'FAIL', 'XPASS'}:
            continue
        text = item['text']
        if 'test for warnings' in text:
            kind = 'warning'
        elif 'test for errors' in text:
            kind = 'required-error-pattern'
        elif 'test for excess errors' in text:
            kind = 'excess-diagnostics'
        elif 'execution test' in text:
            kind = 'execution'
        elif 'output pattern' in text:
            kind = 'runtime-output'
        else:
            kind = 'other-native-assertion'
        counts[kind] += 1
    return dict(counts)


def verify(path, digest):
    if sha256(path) != digest:
        raise ValueError('evidence hash mismatch: ' + str(path))


def summarize(root, allow_incomplete=False):
    manifest = json.loads((root / 'manifest.json').read_text())
    for name, digest in manifest['plan_hashes'].items():
        verify(root / name, digest)
    verify(root / 'candidate', manifest['inputs']['hashes']['candidate'])
    for name, digest in manifest['inputs']['scripts'].items():
        verify(root / 'adapter' / name, digest)
    with (root / 'accounting.jsonl').open() as stream:
        accounting = [json.loads(s) for s in stream]
    with (root / 'tasks.jsonl').open() as stream:
        tasks = [json.loads(s) for s in stream]
    pairs, mode_counts, assertions, rows = {}, defaultdict(Counter), defaultdict(Counter), []
    completed, streams, objects = 0, 0, 0
    for task in tasks:
        state_path = root / 'cells' / task['id'] / 'state.json'
        state = json.loads(state_path.read_text()) if state_path.exists() else None
        if not state or state['state'] != 'terminal':
            if not allow_incomplete:
                raise ValueError('unfinished cell: ' + task['id'])
            continue
        if state['task'] != task:
            raise ValueError('state identity mismatch: ' + task['id'])
        completed += 1
        mode, dialect = task['mode'], task['dialect']
        mode_counts[mode][state['status']] += 1
        data = {}
        if 'result' in state:
            result = Path(state['result'])
            verify(result, state['result_sha256'])
            verify(result.parent / 'manifest.json', state['manifest_sha256'])
            verify(result.parent / 'config.json', state['config_sha256'])
            data = json.loads(result.read_text())
            if data['status'] != state['status']:
                raise ValueError('result status differs from state: ' + task['id'])
            for call in data['calls']:
                for source in call.get('sources', []):
                    verify(source['saved'], source['sha256'])
                    if source['path'] == task['source'] and source['sha256'] != task['sha256']:
                        raise ValueError('executed source differs from task: ' + task['id'])
                for step in call['steps']:
                    for stream in ['stdout', 'stderr']:
                        verify(step[stream + '_path'], step[stream + '_sha256'])
                        streams += 1
                    if 'output' in step:
                        verify(step['output']['saved'], step['output']['sha256'])
                        objects += 1
        failed = failed_assertions(data)
        assertions[mode].update(failed)
        row = dict(identity=task['identity'], dialect=dialect, mode=mode,
                   status=state['status'], task_id=task['id'],
                   native_counts=data.get('native_counts', {}), failed_assertions=failed,
                   reasons=state.get('reasons', [state['reason']] if 'reason' in state else []),
                   state=str(state_path), result=state.get('result'))
        rows.append(row)
        pair = pairs.setdefault((task['identity'], dialect), dict(identity=task['identity'], dialect=dialect))
        pair[mode] = row
    paired = []
    for pair in pairs.values():
        if set(manifest['inputs']['modes']) == {'host'}:
            pair['classification'] = 'host-only-baseline'
        elif set(manifest['inputs']['modes']) == {'candidate'}:
            pair['classification'] = 'candidate-only-observation'
        else:
            pair['classification'] = pair_status(pair.get('host', {}).get('status'),
                                                 pair.get('candidate', {}).get('status'))
        pair['standards_basis'] = 'not-adjudicated-by-execution-runner'
        paired.append(pair)
    return dict(campaign=str(root), profile={key: manifest['inputs'][key]
                for key in ['defaults', 'dialects', 'modes']},
                scheduled_cells=len(tasks), verified_terminal_cells=completed,
                snapshot_complete=completed == len(tasks),
                accounting_counts=dict(Counter(row['status'] for row in accounting)),
                by_mode={mode: dict(counts) for mode, counts in mode_counts.items()},
                pair_counts=dict(Counter(p['classification'] for p in paired)),
                failed_assertions_by_mode={mode: dict(counts) for mode, counts in assertions.items()},
                verified_raw_streams=streams, verified_outputs=objects,
                rows=rows, pairs=paired, full_upstream_pass=False, goal_complete=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--campaign', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--allow-incomplete', action='store_true')
    args = parser.parse_args()
    result = summarize(args.campaign.resolve(), args.allow_incomplete)
    args.output.mkdir(parents=True, exist_ok=False)
    write_json(args.output / 'results.json', result)
    with (args.output / 'pairs.csv').open('w', newline='') as stream:
        fields = ['identity', 'dialect', 'host_status', 'candidate_status', 'classification', 'host_result', 'candidate_result']
        writer = csv.DictWriter(stream, fields); writer.writeheader()
        for pair in result['pairs']:
            host, candidate = pair.get('host', {}), pair.get('candidate', {})
            writer.writerow(dict(identity=pair['identity'], dialect=pair['dialect'], classification=pair['classification'],
                                 host_status=host.get('status'), candidate_status=candidate.get('status'),
                                 host_result=host.get('result'), candidate_result=candidate.get('result')))
    print(json.dumps({key: value for key, value in result.items() if key not in {'rows', 'pairs'}}))
    return 0


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Independently verify frozen Clang language receipts and exact accounting."""
import argparse
from collections import Counter
import importlib.util
import json
from pathlib import Path

from upstream_dg_proxy import sha256


def verify_file(path, expected):
    if sha256(path) != expected:
        raise ValueError('Evidence hash mismatch: ' + str(path))


def verify_campaign(root):
    manifest = json.loads((root / 'manifest.json').read_text())
    for mapping in ['inputs', 'native_helpers', 'binaries']:
        for path, digest in manifest[mapping].items():
            verify_file(path, digest)
    for name, digest in manifest['plan_hashes'].items():
        verify_file(root / name, digest)
    metadata = [json.loads(line) for line in (root / 'inputs/metadata.jsonl').read_text().splitlines()]
    identities = {row['identity']: row for row in metadata}
    if len(identities) != len(metadata):
        raise ValueError('Duplicate inventory identity')
    initial = json.loads((root / 'accounting-initial.json').read_text())
    if len(initial) != len(metadata) or {row['identity'] for row in initial} != set(identities):
        raise ValueError('Inventory accounting is incomplete')
    selected = {row['identity'] for row in initial if row['selected']}
    if selected not in [set(identities), {row['identity'] for row in metadata if row['selected']}]:
        raise ValueError('Selected scope silently differs from declared all/original cohort')
    for row in initial:
        source = identities[row['identity']]
        if row['sha256'] != source['sha256'] or row['declared_runs'] != len(source['commands']):
            raise ValueError('Accounting source/variant identity mismatch')
    dialects = manifest['dialects']
    if not dialects or len(set(dialects)) != len(dialects):
        raise ValueError('Missing/duplicate dialects')
    expected = {(row['identity'], index, dialect): (row['sha256'], command)
                for row in metadata if row['identity'] in selected
                for index, command in enumerate(row['commands']) for dialect in dialects}
    tasks = json.loads((root / 'tasks.json').read_text())
    key = lambda row: (row['identity'], row['run_index'], row['dialect'])
    if len(tasks) != len(expected) or {key(row) for row in tasks} != set(expected):
        raise ValueError('Scheduled variants differ from selected metadata')
    rows = json.loads((root / 'results.json').read_text())
    if len(rows) != len(expected) or len({key(row) for row in rows}) != len(expected):
        raise ValueError('Terminal variant accounting is incomplete or duplicated')
    if any(row.get('full_upstream_pass') is not False for row in rows):
        raise ValueError('Projected obligation is incorrectly labeled a full upstream pass')
    specification = importlib.util.spec_from_file_location('frozen_clang_language', root / 'adapter/upstream_clang_language.py')
    frozen = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(frozen)
    adjudications = json.loads((root / 'inputs/adjudications.json').read_text())
    streams, objects, snapshots, assessed = 0, 0, 0, 0
    for row in rows:
        identity = key(row)
        if identity not in expected or (row['source_sha256'], row['original_run']) != expected[identity]:
            raise ValueError('Result differs from planned source/variant')
        if row.get('full_upstream_pass') is not False:
            raise ValueError('Projected obligation is incorrectly labeled a full upstream pass')
        stages = row.get('stages', [])
        by_name = {stage['name']: stage for stage in stages}
        if len(by_name) != len(stages):
            raise ValueError('Duplicate stage name')
        for stage in stages:
            for stream in ['stdout', 'stderr']:
                verify_file(stage[stream + '_path'], stage[stream + '_sha256'])
                streams += 1
            if stage.get('output'):
                verify_file(stage['output']['path'], stage['output']['sha256'])
                if not frozen.elf_object(stage['output']['path']):
                    raise ValueError('Successful object is not an x86-64 ELF relocatable')
                objects += 1
        if stages:
            cell = Path(stages[0]['stdout_path']).parent
            verify_file(cell / 'source.snapshot', row['source_sha256'])
            snapshots += 1
            saved = json.loads((cell / 'receipt.json').read_text())
            if saved != row:
                raise ValueError('Per-cell receipt differs from campaign result')
            source = str(Path(manifest['llvm_root']) / row['identity'].removeprefix('llvm-project/'))
            projection = row['projection']
            commands = {
                'candidate-preprocess': [str(root / 'candidate'), *projection['candidate_args'],
                                         '-E', source, '-o', str(cell / 'candidate.tokens')],
                'native-expectations': [str(root / 'helper'), str(cell / 'context.json')],
                'host-object': [str(root / 'clang'), '-cc1', *projection['native_args'],
                                '-emit-obj', source, '-o', str(cell / 'host.o')],
                'candidate-object': [str(root / 'candidate'), *projection['candidate_args'],
                                     '-c', source, '-o', str(cell / 'candidate.o')],
            }
            for name, stage in by_name.items():
                if name not in commands or stage['command'] != commands[name]:
                    raise ValueError('Compiler/helper invocation differs from the declared projection')
        if row.get('native_context'):
            verify_file(row['native_context']['path'], row['native_context']['sha256'])
        if 'native-expectations' in by_name:
            data = json.loads(Path(by_name['native-expectations']['stdout_path']).read_text())
            if row.get('discovery') != data:
                raise ValueError('Stored discovery differs from raw native helper output')
        if row.get('candidate_token_stream'):
            token_stream = row['candidate_token_stream']
            verify_file(token_stream['path'], token_stream['sha256'])
        for path, digest in row.get('native_source_closure', {}).items():
            verify_file(path, digest)
        if 'host-object' in by_name and 'candidate-object' in by_name:
            adjudication_key = row['identity'] + '#' + str(row['run_index']) + '#' + row['dialect']
            adjudication = adjudications.get(adjudication_key)
            if adjudication and adjudication['source_sha256'] != row['source_sha256']:
                raise ValueError('Adjudication source differs from executed source')
            result = frozen.assess(row['discovery'], by_name['host-object'], by_name['candidate-object'],
                            row['projection'], adjudication)
            for name, value in result.items():
                if row.get(name) != value:
                    raise ValueError('Independent language assessment differs: ' + str(identity) + ' ' + name)
            assessed += 1
    summary = json.loads((root / 'summary.json').read_text())
    if summary['outcomes'] != dict(Counter(row['status'] for row in rows)) or summary['variants'] != len(expected):
        raise ValueError('Summary outcomes differ from complete terminal accounting')
    state = json.loads((root / 'state.json').read_text())
    if state != dict(state='complete', scheduled_variants=len(expected), terminal_variants=len(expected)):
        raise ValueError('Campaign is not authoritatively terminal/complete')
    return dict(passed=True, selected_sources=len(selected), inventory_sources=len(metadata),
                terminal_variants=len(rows), rehashed_streams=streams, rehashed_objects=objects,
                rehashed_source_snapshots=snapshots, independently_reassessed_object_variants=assessed,
                outcomes=summary['outcomes'], full_upstream_pass=False,
                scope='Integrity/accounting and projected-language assessment only; remaining native configuration, output/runtime checks, standards adjudications and corpus execution remain incomplete')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--campaign', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = verify_campaign(args.campaign.resolve())
    result['verifier_sha256'] = sha256(Path(__file__))
    with args.output.open('x') as stream:
        json.dump(result, stream, indent=2)
        stream.write('\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()

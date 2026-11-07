#!/usr/bin/env python3
"""Qualify implicit-mode projections separately from paired language grading.

The strict native run never changes a case's expected result or selects cases
using cppgm outcomes. A failure remains an applicability/standard-specific
triage observation; the ordinary paired run still executes the fixed manifest.
"""
import argparse
import json
from pathlib import Path

from run_cxx11_ci_suite import describe, run_suite, validate_manifest
from upstream_dg_proxy import sha256, write_json


def qualification_cases(manifest):
    cases = []
    for case in manifest['cases']:
        policy = case.get('qualification')
        if policy is None:
            continue
        if policy['kind'] != 'implicit-mode-cxx11' or policy['additional_upstream_flags'] != ['-pedantic-errors']:
            raise ValueError('Unknown strict native qualification policy')
        cases.append(dict(case, upstream_flags=[*case['upstream_flags'], '-pedantic-errors']))
    return cases


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--upstream', required=True)
    parser.add_argument('--upstream-version', required=True)
    parser.add_argument('--workers', type=int, default=2)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args(argv)
    if args.workers < 1:
        parser.error('workers must be positive')
    root = args.source_root.resolve()
    manifest = json.loads(args.manifest.read_text())
    validate_manifest(manifest, root)
    if manifest['suite'] != 'clang':
        raise ValueError('Native Clang qualification requires a Clang manifest')
    binary = describe(args.upstream)
    if args.upstream_version not in binary['version']:
        raise ValueError('Upstream compiler does not match pinned suite version')
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    cases = qualification_cases(manifest)
    write_json(output / 'qualification-manifest.json', dict(manifest, cases=cases))
    result = run_suite(cases, root, binary['path'], binary['path'], 'upstream',
                       output / 'strict-native', args.workers)
    validate_manifest(manifest, root)
    complete = result['executed'] == len(cases) and result['error'] is None
    passed = complete and result['passed'] == len(cases)
    scripts = Path(__file__).resolve().parent
    report = dict(schema=1, suite='clang', passed=passed, complete=complete,
                  manifest_sha256=sha256(args.manifest), binary=binary, workers=args.workers,
                  runner_scripts={n: sha256(scripts / n) for n in [
                      'qualify_clang_cxx11_ci_manifest.py', 'run_cxx11_ci_suite.py',
                      'upstream_dg_proxy.py', 'upstream_clang_language.py']},
                  result=result,
                  interpretation='Strict native qualification only. Failures require applicability/standard '
                                 'triage; do not infer cppgm defects, promote warnings to mandatory errors, '
                                 'or automatically change positive expectations. The normal paired run '
                                 'must still execute the unchanged fixed manifest.')
    write_json(output / 'results.json', report)
    (output / 'summary.md').write_text(
        '### Clang implicit-mode C++11 qualification\n\n'
        f"Strict native: {result['passed']} / {len(cases)}; executed {result['executed']}.\n\n"
        + report['interpretation'] + '\n')
    print(json.dumps(dict(passed=passed, qualified=result['passed'], selected=len(cases),
                          executed=result['executed'], output=str(output))))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())

#!/usr/bin/env python3
"""Publish the successful strict assignment report with its CI provenance."""
import argparse
import json
import os
from pathlib import Path
import re

from publish_benchmark_badges import badge_svg


def record(log, environment):
    matches = re.findall(r'^===== ALL TESTS PASSED SUCCESSFULLY! \((\d+) / (\d+)\) =====$',
                         log, re.MULTILINE)
    if not matches:
        raise ValueError('missing successful final assignment summary')
    passed, total = map(int, matches[-1])
    if passed != total or total <= 0:
        raise ValueError('assignment report is not a complete success')
    revision = environment['GITHUB_SHA']
    if not re.fullmatch('[0-9a-f]{40}', revision):
        raise ValueError('source revision must be a full commit hash')
    return dict(passed=passed, total=total, revision=revision,
                run_number=int(environment['GITHUB_RUN_NUMBER']),
                run_attempt=int(environment['GITHUB_RUN_ATTEMPT']),
                run_url=(environment['GITHUB_SERVER_URL'] + '/' +
                         environment['GITHUB_REPOSITORY'] + '/actions/runs/' +
                         environment['GITHUB_RUN_ID']))


def publish(result, target, revision):
    if result['revision'] != revision:
        raise ValueError('artifact source revision differs from this workflow')
    if result['passed'] != result['total'] or result['total'] <= 0:
        raise ValueError('only a complete successful report may be published')
    directory = Path(target) / 'internal-tests'
    previous = directory / 'results.json'
    order = lambda value: (value['run_number'], value['run_attempt'])
    if previous.exists() and order(json.loads(previous.read_text())) > order(result):
        return False
    directory.mkdir(parents=True, exist_ok=True)
    previous.write_text(json.dumps(result, indent=2) + '\n')
    count = f"{result['passed']:,} / {result['total']:,}"
    (directory / 'tests.svg').write_text(badge_svg(dict(
        label='Internal assignment tests', message=count, color='15803d')))
    source = ('https://github.com/vishvananda/cppgm-extended/commit/' + revision)
    (directory / 'report.md').write_text(
        '# Internal assignment tests\n\n' + count + ' passed.\n\n' +
        f"Compiler: [`{revision[:9]}`]({source}) · [CI report]({result['run_url']})\n\n" +
        'This is the strict `make test-report-nobuild` run with GCC on Ubuntu 26.04. '
        'The count covers the required assignment suites on Linux. Optional target '
        'features, debug information, macOS and self-host checks have separate CI jobs.\n')
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    item = sub.add_parser('record')
    item.add_argument('log', type=Path)
    item.add_argument('output', type=Path)
    item = sub.add_parser('publish')
    item.add_argument('artifact', type=Path)
    item.add_argument('target', type=Path)
    item.add_argument('--revision', required=True)
    args = parser.parse_args()
    if args.command == 'record':
        args.output.write_text(json.dumps(record(args.log.read_text(), os.environ), indent=2) + '\n')
    else:
        publish(json.loads(args.artifact.read_text()), args.target, args.revision)


if __name__ == '__main__':
    main()

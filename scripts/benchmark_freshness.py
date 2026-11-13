#!/usr/bin/env python3
"""Refresh benchmark labels and revision age without changing measurements."""
import argparse
from html import escape
import json
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET

from library_benchmark_badges import HOSTS
from publish_benchmark_badges import badge_svg

STALE_COMMITS = 10
NS = '{http://www.w3.org/2000/svg}'


def comparison(repo, measured, main):
    # Page two omits the potentially large file diff. One request per distinct
    # measured revision, never per badge; the response still includes counts.
    endpoint = f'repos/{repo}/compare/{measured}...{main}?per_page=1&page=2'
    data = json.loads(subprocess.check_output(['gh', 'api', endpoint], text=True))
    status = data['status']
    return dict(commits_behind=data['ahead_by'] if status in ('ahead', 'identical') else None,
                status=status, measured_at=data['base_commit']['commit']['committer']['date'])


def groups(target):
    for host in HOSTS:
        directory = target / host
        public = [directory / f'{level}-{phase}{reference}.svg' for level in ('o0', 'o3')
                  for phase in ('compile', 'generated') for reference in ('', '-clang')]
        builds = [directory / (name + '.svg') for name in
                  ('self-compile', 'self-built', 'self-compile-clang', 'self-built-clang')]
        yield 'Public programs', host, directory / 'provenance.json', (p for p in public if p.exists()), directory / 'report.md'
        yield 'cppgm++', host, directory / 'self-build-provenance.json', (p for p in builds if p.exists()), directory / 'self-build.md'
        for workload in ('index-cpp', 'simdjson', 'godot', 'boost/o3', 'internal', 'wasm', 'wasm-selfbuild'):
            directory = target / workload
            yield workload, host, directory / host / 'metadata.json', directory.glob(host + '-*.svg'), directory / host / 'report.md'


def render(path, caption, metadata, age):
    root = ET.fromstring(path.read_text())
    texts = root.findall(NS + 'text')
    message = root.get('data-original-message', texts[-1].text or '')
    label = root.get('data-original-label')
    if label is None:
        label = (root.findtext(NS + 'title') or root.get('aria-label', '')).removesuffix(': ' + message)
    panels = root.findall('.//' + NS + 'rect')
    color = panels[-1].get('fill', '#475569').removeprefix('#')
    measured = metadata is not None and message not in ('n/m', 'n/a', 'no data')
    note, stale = '', False
    if measured:
        count = age['commits_behind']
        stale = count is None or count >= STALE_COMMITS
        detail = f'{count} commits behind main' if count is not None else 'outside main history'
        cpu = '' if metadata['cpu'] in label else '; ' + metadata['cpu']
        note = f"{cpu}; measured {metadata['revision']}; {detail}"
    data = dict(label=label + note, left_label=caption,
                message=message + ('*' if stale else ''), color=color)
    content = badge_svg(data)
    attributes = (f' data-original-label="{escape(label, quote=True)}"'
                  f' data-original-message="{escape(message, quote=True)}"')
    return content.replace('<svg ', '<svg' + attributes + ' ', 1)


def refresh(target, main, repo, compare=comparison):
    if not re.fullmatch(r'[0-9a-f]{40}', main):
        raise ValueError('freshness requires the full main revision')
    cache_path = target / 'freshness.json'
    previous = json.loads(cache_path.read_text()) if cache_path.exists() else {}
    ages, old = {}, previous.get('revisions', {})
    if previous.get('main') == main:
        ages = dict(old)
    elif previous.get('main'):
        delta = compare(repo, previous['main'], main)
        if delta['commits_behind'] is not None:
            # Reachable commits grow monotonically across a fast-forward.
            # One comparison advances every known ancestor, including merges.
            ages = {revision: dict(value, commits_behind=value['commits_behind'] + delta['commits_behind'],
                                   status='ahead' if value['commits_behind'] + delta['commits_behind'] else 'identical')
                    for revision, value in old.items() if value['commits_behind'] is not None}
    rows, replacements = [], {}
    for workload, host, metadata_path, paths, report in groups(target):
        metadata = json.loads(metadata_path.read_text()) if metadata_path.exists() else None
        age = None
        if metadata:
            revision = metadata['revision']
            if not re.fullmatch(r'[0-9a-f]{40}', revision):
                raise ValueError('invalid measured revision: ' + revision)
            if revision not in ages:
                ages[revision] = compare(repo, revision, main)
            age = ages[revision]
            rows.append(dict(workload=workload, host=host, revision=revision,
                             cpu=metadata['cpu'], report=str(report.relative_to(target)), **age))
        for path in sorted(paths):
            replacements[path] = render(path, HOSTS[host], metadata, age)
    # Validate every source and comparison before changing the retained badges.
    for path, text in replacements.items():
        path.write_text(text)
    cache_path.write_text(json.dumps(dict(schema=1, main=main, stale_commits=STALE_COMMITS,
                                         revisions=ages, measurements=rows), indent=2) + '\n')
    lines = ['# Benchmark measurement age', '',
             f'Compared with main [`{main[:9]}`](https://github.com/{repo}/commit/{main}).', '',
             f'An asterisk marks a result at least {STALE_COMMITS} commits behind main, or outside its history. '
             'Commit distance includes documentation and workflow changes; it does not establish a performance change. '
             'Missing measurements remain `n/m`. The metric value and its performance color are retained.', '',
             '| Workload | Host | CPU | Measured commit | Commit date (UTC) | Age | Results |',
             '|---|---|---|---|---|---|---|']
    for row in rows:
        count = row['commits_behind']
        age = f'{count} commits' if count is not None else 'outside main history'
        revision = row['revision']
        cpu = row['cpu'].replace('|', '\\|').replace('\n', ' ')
        lines.append(f"| {row['workload']} | {HOSTS[row['host']]} | {cpu} | "
                     f"[`{revision[:9]}`](https://github.com/{repo}/commit/{revision}) | "
                     f"{row['measured_at']} | {age} | [Results]({row['report']}) |")
    (target / 'freshness.md').write_text('\n'.join(lines) + '\n')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('target', type=Path)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--repo', default='vishvananda/cppgm-extended')
    args = parser.parse_args()
    from wasm_benchmark_results import seed
    seed(args.target)
    refresh(args.target, args.revision, args.repo)


if __name__ == '__main__':
    main()

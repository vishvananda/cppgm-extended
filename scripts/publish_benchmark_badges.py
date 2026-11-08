#!/usr/bin/env python3
"""Update one CPU's badges while retaining the other CPU's last measurement."""
import argparse
import datetime
from html import escape
import json
from pathlib import Path
import re

import public_benchmarks
import benchmark_hosts

BADGES = ('o0-compile', 'o0-generated', 'o3-compile', 'o3-generated',
          'self-compile', 'self-built')
CLANG_BADGES = ('self-compile-clang', 'self-built-clang')


def badge_svg(data):
    """Keep bold performance text in a compact, rounded badge."""
    message = data['message']
    color = data['color']
    if not re.fullmatch(r'[0-9a-fA-F]{6}', color):
        raise ValueError('badge color must be a six-digit hex value')
    channels = [int(color[n:n+2], 16) / 255 for n in (0, 2, 4)]
    linear = [v / 12.92 if v <= .04045 else ((v + .055) / 1.055) ** 2.4 for v in channels]
    luminance = sum(v * weight for v, weight in zip(linear, (.2126, .7152, .0722)))
    text_color = '#000' if luminance > .179 else '#fff'
    left = data.get('left_label', '')
    width = 120
    left_width = 64 if left else 0
    message_width = width - left_width
    text_width = min(max(8, len(message) * 8 - 1), message_width - 12)
    left_text_width = min(max(8, len(left) * 8 - 1), max(8, left_width - 12))
    background = f'  <rect width="{width}" height="24" rx="4" fill="#{color}"/>\n'
    left_text = ''
    if left:
        background = (f'  <defs><clipPath id="badge"><rect width="{width}" height="24" rx="4"/></clipPath></defs>\n'
                      '  <g clip-path="url(#badge)">\n'
                      f'    <rect width="{left_width}" height="24" fill="#555"/>\n'
                      f'    <rect x="{left_width}" width="{message_width}" height="24" fill="#{color}"/>\n'
                      '  </g>\n')
        left_text = (f'  <text x="{left_width / 2:g}" y="16.5" fill="#fff" text-anchor="middle" '
                     'font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="12" '
                     f'font-weight="700" textLength="{left_text_width}" '
                     f'lengthAdjust="spacingAndGlyphs">{escape(left)}</text>\n')
    title = escape(f"{data['label']}: {message}", quote=True)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="24" '
            f'viewBox="0 0 {width} 24" role="img" aria-label="{title}">\n'
            f'  <title>{title}</title>\n'
            f'{background}{left_text}'
            f'  <text x="{left_width + message_width / 2:g}" y="16.5" fill="{text_color}" text-anchor="middle" '
            'font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="12" '
            f'font-weight="700" letter-spacing="1" textLength="{text_width}" '
            f'lengthAdjust="spacingAndGlyphs">{escape(message)}</text>\n'
            '</svg>\n')


def prepare(artifact, target, platform, revision, run_url, legacy_epyc=False, component='all'):
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('the measured revision must be a full commit hash')
    if component not in ('all', 'public', 'self-build'):
        raise ValueError('unknown benchmark component')
    artifact, target = Path(artifact), Path(target)
    document = None
    builds = None
    if component != 'self-build':
        document = json.loads((artifact / 'results.json').read_text())
    if component != 'public' and (artifact / 'selfhost.json').is_file():
        builds = json.loads((artifact / 'selfhost.json').read_text())
    if component == 'self-build':
        if not builds:
            raise ValueError('self-build publication requires selfhost.json')
        environment = builds['metadata']
        if environment.get('revision') != revision:
            raise ValueError('self-build artifact does not match the measured revision')
    else:
        environment = document['environment']
    cpu = environment.get('cpu') or ''
    if platform == 'github':
        platform = benchmark_hosts.hosted_platform(cpu)
    expected = 'Intel' if platform in ('intel', 'github-intel') else 'AMD EPYC'
    if expected not in cpu:
        raise ValueError(f'{platform} badges require {expected} hardware; measured {cpu!r}')
    names = (BADGES[:4] if component == 'public' else
             BADGES[4:] + CLANG_BADGES if component == 'self-build' else BADGES + CLANG_BADGES)
    # Validate and render everything before replacing any previous measurement.
    badges = {}
    for name in names:
        path = artifact / 'badges' / f'{name}.json'
        data = (dict(schemaVersion=1, label=name, message='no data', color='475569')
                if name in CLANG_BADGES and not path.exists() else json.loads(path.read_text()))
        if data.get('schemaVersion') != 1 or not isinstance(data.get('message'), str):
            raise ValueError(f'invalid badge: {name}')
        badges[name] = data
    destinations = [target / platform]
    if platform == 'epyc' and legacy_epyc:
        destinations.append(target)
    provenance = dict(revision=revision, cpu=cpu, run_url=run_url,
                      published_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      environment=environment)
    revision_badge = dict(schemaVersion=1, label='commit', message=revision[:9], color='1e40af')
    if document is not None:
        badges['revision'] = revision_badge
        _, count = public_benchmarks.mean_ratio(document['programs'], 'run_cpu_O3', 'gcc')
        badges['o3-programs'] = dict(schemaVersion=1, label='programs', message=str(count), color='475569')
    if builds is not None:
        badges['self-revision'] = revision_badge
    rendered = {name: badge_svg(data) for name, data in badges.items()}
    heading = {'intel': 'Intel Xeon (dedicated)', 'github-intel': 'Intel (GitHub)', 'epyc': 'AMD EPYC'}[platform]
    source = 'https://github.com/vishvananda/cppgm-extended/commit/' + revision
    note = (f'Measured compiler: [`{revision[:9]}`]({source}) · [CI run]({run_url})')
    files = {}
    if document is not None:
        report = public_benchmarks.summary(document, [], builds)
        report = report.replace('# Public benchmarks\n\n',
                                f'# Public benchmarks: {heading}\n\n' + note
                                + ' · [Raw program results](results.json)'
                                + (' · [Raw rebuild results](selfhost.json)' if builds else '')
                                + '\n\n', 1)
        report += ('\n## Toolchains\n\n' + '\n'.join(
            f'- **{name}**: {value}' for name, value in environment.items() if name != 'cpu') + '\n')
        report += '\n[Compiler build and codegen results](self-build.md) have their own measured revision.\n'
        files.update({'provenance.json': json.dumps(provenance, indent=1) + '\n',
                      'report.md': report, 'results.json': (artifact / 'results.json').read_text()})
    if builds is not None:
        report = public_benchmarks.selfhost_summary(builds)
        report = report.replace('# Compiler build and codegen benchmarks\n\n',
                                f'# Compiler build and codegen benchmarks: {heading}\n\n' + note
                                + ' · [Raw rebuild results](selfhost.json)\n\n', 1)
        report += '\n[Public program results](report.md) have their own measured revision.\n'
        files.update({'self-build-provenance.json': json.dumps(provenance, indent=1) + '\n',
                      'self-build.md': report, 'selfhost.json': (artifact / 'selfhost.json').read_text()})
    for destination in destinations:
        destination.mkdir(parents=True, exist_ok=True)
        for name, data in badges.items():
            (destination / f'{name}.json').write_text(json.dumps(data, indent=1) + '\n')
            (destination / f'{name}.svg').write_text(rendered[name])
        for name, content in files.items():
            (destination / name).write_text(content)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('artifact', type=Path)
    parser.add_argument('target', type=Path)
    parser.add_argument('--platform', choices=['github', 'epyc', 'intel', 'github-intel'], required=True)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--run-url', required=True)
    parser.add_argument('--legacy-epyc', action='store_true', help='also update main\'s original EPYC URLs')
    parser.add_argument('--component', choices=['all', 'public', 'self-build'], default='all',
                        help='update only this benchmark component, retaining the other measurements')
    args = parser.parse_args()
    prepare(args.artifact, args.target, args.platform, args.revision, args.run_url, args.legacy_epyc, args.component)


if __name__ == '__main__':
    main()

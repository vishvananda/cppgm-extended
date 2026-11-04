#!/usr/bin/env python3
"""Update one CPU's badges while retaining the other CPU's last measurement."""
import argparse
import datetime
from html import escape
import json
from pathlib import Path
import re

import public_benchmarks

BADGES = ('o0-compile', 'o0-generated', 'o3-compile', 'o3-generated',
          'self-compile', 'self-built')


def badge_svg(data):
    """Keep bold performance text in a compact, rounded badge."""
    message = data['message']
    color = data['color']
    if not re.fullmatch(r'[0-9a-fA-F]{6}', color):
        raise ValueError('badge color must be a six-digit hex value')
    text_width = max(8, len(message) * 8 - 1)
    width = text_width + 12
    title = escape(f"{data['label']}: {message}", quote=True)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="24" '
            f'viewBox="0 0 {width} 24" role="img" aria-label="{title}">\n'
            f'  <title>{title}</title>\n'
            f'  <rect width="{width}" height="24" rx="4" fill="#{color}"/>\n'
            f'  <text x="{width / 2:g}" y="16.5" fill="#fff" text-anchor="middle" '
            'font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="12" '
            f'font-weight="700" letter-spacing="1" textLength="{text_width}" '
            f'lengthAdjust="spacingAndGlyphs">{escape(message)}</text>\n'
            '</svg>\n')


def prepare(artifact, target, platform, revision, run_url, legacy_epyc=False):
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('the measured revision must be a full commit hash')
    artifact, target = Path(artifact), Path(target)
    document = json.loads((artifact / 'results.json').read_text())
    environment = document['environment']
    cpu = environment.get('cpu') or ''
    expected = 'Intel' if platform == 'intel' else 'AMD EPYC'
    if expected not in cpu:
        raise ValueError(f'{platform} badges require {expected} hardware; measured {cpu!r}')
    # Validate the whole set before replacing any previous measurement.
    badges = {}
    for name in BADGES:
        data = json.loads((artifact / 'badges' / f'{name}.json').read_text())
        if data.get('schemaVersion') != 1 or not isinstance(data.get('message'), str):
            raise ValueError(f'invalid badge: {name}')
        badges[name] = data
    destinations = [target / platform]
    # Only main's automatic run updates the original, unqualified URLs.
    if platform == 'epyc' and legacy_epyc:
        destinations.append(target)
    provenance = dict(revision=revision, cpu=cpu, run_url=run_url,
                      published_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                      environment=environment)
    badges['revision'] = dict(schemaVersion=1, label='commit', message=revision[:9], color='1e40af')
    _, count = public_benchmarks.mean_ratio(document['programs'], 'run_cpu_O3', 'gcc')
    badges['o3-programs'] = dict(schemaVersion=1, label='programs', message=str(count), color='475569')
    builds = None
    if (artifact / 'selfhost.json').is_file():
        builds = json.loads((artifact / 'selfhost.json').read_text())
    report = public_benchmarks.summary(document, [], builds)
    heading = 'Intel Xeon' if platform == 'intel' else 'AMD EPYC'
    source = 'https://github.com/vishvananda/cppgm-extended/commit/' + revision
    report = report.replace('# Public benchmarks\n\n',
                            f'# Public benchmarks: {heading}\n\n'
                            f'Measured compiler: [`{revision[:9]}`]({source}) · '
                            f'[CI run]({run_url}) · [Raw program results](results.json)'
                            + (' · [Raw rebuild results](selfhost.json)' if builds else '') + '\n\n', 1)
    report += ('\n## Toolchains\n\n' + '\n'.join(
        f'- **{name}**: {value}' for name, value in environment.items() if name != 'cpu') + '\n')
    for destination in destinations:
        destination.mkdir(parents=True, exist_ok=True)
        for name, data in badges.items():
            (destination / f'{name}.json').write_text(json.dumps(data, indent=1) + '\n')
            (destination / f'{name}.svg').write_text(badge_svg(data))
        (destination / 'provenance.json').write_text(json.dumps(provenance, indent=1) + '\n')
        (destination / 'report.md').write_text(report)
        (destination / 'results.json').write_bytes((artifact / 'results.json').read_bytes())
        if builds:
            (destination / 'selfhost.json').write_bytes((artifact / 'selfhost.json').read_bytes())
        else:
            (destination / 'selfhost.json').unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('artifact', type=Path)
    parser.add_argument('target', type=Path)
    parser.add_argument('--platform', choices=['epyc', 'intel'], required=True)
    parser.add_argument('--revision', required=True)
    parser.add_argument('--run-url', required=True)
    parser.add_argument('--legacy-epyc', action='store_true', help='also update main\'s original EPYC URLs')
    args = parser.parse_args()
    prepare(args.artifact, args.target, args.platform, args.revision, args.run_url, args.legacy_epyc)


if __name__ == '__main__':
    main()

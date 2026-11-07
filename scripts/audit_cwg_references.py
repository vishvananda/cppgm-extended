#!/usr/bin/env python3
"""Check that explicit course CWG references have offline student documents."""
import hashlib
import json
from pathlib import Path
import re


def mentioned_issues(text):
    numbers = set()
    pattern = (r'\bCWG\s*(?:issues?\s*)?#?\s*(\d+)\b(?!\.\d)'
               r'((?:\s*(?:/|,|and\s*)\s*\d+\b(?!\.\d))*)')
    for match in re.finditer(pattern, text, re.I):
        numbers.add(int(match.group(1)))
        numbers.update(map(int, re.findall(r'\d+', match.group(2))))
    numbers.update(map(int, re.findall(
        r'https://cplusplus\.github\.io/CWG/issues/(\d+)\.html', text)))
    return numbers


def main():
    root = Path(__file__).resolve().parents[1]
    directory = root / 'doc/cwg'
    manifest = json.loads((directory / 'manifest.json').read_text())
    approved = set((root / 'scripts/student_export_documents.txt').read_text().splitlines())
    covered, errors = set(), []
    for record in manifest['issues'] + manifest['supplemental_papers']:
        name = record['file']
        path = directory / name
        if path.name != name or not name.endswith('.txt'):
            errors.append('Invalid text filename: ' + name)
            continue
        if not path.is_file():
            errors.append('Missing reference: ' + name)
            continue
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        if digest != record['text_sha256']:
            errors.append('Changed reference without updated provenance: ' + name)
        if len(path.read_text().splitlines()) < 15:
            errors.append('Reference has no substantive source text: ' + name)
        if 'Source: ' + record['source_url'] not in path.read_text():
            errors.append('Missing source attribution: ' + name)
        if 'doc/cwg/' + name not in approved:
            errors.append('Reference missing from student export: ' + name)
        if 'issue' in record:
            if record['issue'] in covered:
                errors.append('Duplicate issue: ' + str(record['issue']))
            covered.add(record['issue'])
            for paper in record.get('local_papers', []):
                if not (directory / paper).is_file():
                    errors.append('Missing adopted paper: ' + paper)
    for name in ['INDEX.txt', 'manifest.json']:
        if not (directory / name).is_file() or 'doc/cwg/' + name not in approved:
            errors.append('Missing exported catalog file: ' + name)
    paths = [*root.glob('dev/**/*.cpp'), *root.glob('dev/**/*.h'),
             *root.glob('pa*/README.md'), *root.glob('pa*/tests/**/*.t')]
    referenced = set()
    for path in paths:
        numbers = mentioned_issues(path.read_text(errors='replace'))
        referenced.update(numbers)
        for number in sorted(numbers - covered):
            errors.append(f'{path.relative_to(root)}: CWG{number} has no offline text')
    if errors:
        print('\n'.join(errors))
        return 1
    print(f'CWG reference audit: PASS ({len(covered)} issue texts, '
          f'{len(manifest["supplemental_papers"])} papers, '
          f'{len(referenced)} distinct course references)')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())

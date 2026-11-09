"""Keep hosted Intel measurements separate from the dedicated Intel runner."""
import json


def hosted_platform(cpu):
    normalized = cpu.casefold()
    if 'amd epyc' in normalized:
        return 'epyc'
    if 'intel' in normalized:
        return 'github-intel'
    raise ValueError(f'unrecognized hosted benchmark CPU: {cpu!r}')


def library_index(directory, title, metadata_name='metadata.json', report_name='report.md', index_name='report.md'):
    rows = []
    for platform, label in [('epyc', 'EPYC (CI)'), ('github-intel', 'Intel (CI)'),
                            ('intel', 'Intel (dedicated)')]:
        metadata = directory / platform / metadata_name
        if not metadata.is_file():
            continue
        data = json.loads(metadata.read_text())
        cpu = data['cpu'].replace('|', '\\|').replace('\n', ' ')
        rows.append(f"| {label} | {cpu} | `{data['revision'][:9]}` | [Results]({platform}/{report_name}) |")
    text = (f'# {title}\n\nEach host retains its last measurement. Compare the CPU model and measured commit.\n\n'
            '| Host | CPU | Measured compiler | Report |\n|---|---|---|---|\n')
    (directory / index_name).write_text(text + '\n'.join(rows) + '\n')

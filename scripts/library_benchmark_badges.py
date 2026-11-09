"""Render live README badges from the retained, validated benchmark results."""
import json
import math
from pathlib import Path

import benchmark_hosts
import public_benchmarks
from publish_benchmark_badges import badge_svg

HOSTS = {'epyc': 'EPYC', 'intel': 'Intel', 'github-intel': 'CI Intel'}


def geometric(values):
    values = list(values)
    if not values or any(not math.isfinite(v) or v <= 0 for v in values):
        raise ValueError('missing positive benchmark speedups')
    return math.exp(sum(math.log(v) for v in values) / len(values))


def metrics(workload, document):
    """Use the same paired estimates and equal-workload weighting as each report."""
    values = {}
    for reference in ('gcc', 'clang'):
        if workload == 'godot':
            builds = document['compilation']
            values['build-' + reference] = builds[reference]['cpu'] / builds['cppgm']['cpu']
            values['runtime-' + reference] = geometric(
                row[reference]['cpu']['speed'] for row in document['runtime'].values())
        elif workload == 'simdjson':
            for mode in ('scalar', 'sse'):
                row = document['modes'][mode]
                values[mode + '-compile-' + reference] = row['compilation'][reference]['cpu']['speed']
                values[mode + '-runtime-' + reference] = geometric(
                    test[reference]['cpu']['speed'] for test in row['runtime'].values())
        elif workload == 'index-cpp':
            values['compile-' + reference] = document['compilation'][reference]['cpu']['speed']
            values['runtime-' + reference] = document['runtime']['pairs'][reference + ':all']['geometric_speed']
        else:
            raise ValueError('unknown library benchmark: ' + workload)
    if any(not math.isfinite(v) or v <= 0 for v in values.values()):
        raise ValueError('invalid benchmark speedup')
    return values


def svg(label, caption, value):
    return badge_svg(dict(label=label, left_label=caption,
                          message=f'{value:.2f}×' if value is not None else 'n/m',
                          color=public_benchmarks.badge_color(value) if value is not None else '475569'))


def library_badges(directory, workload):
    modes = ('scalar-', 'sse-') if workload == 'simdjson' else ('',)
    compile_phase = 'build' if workload == 'godot' else 'compile'
    names = [mode + phase + '-' + ref for mode in modes
             for phase in (compile_phase, 'runtime') for ref in ('gcc', 'clang')]
    for host, caption in HOSTS.items():
        path = directory / host / 'results.json'
        values, provenance = {}, ''
        if path.is_file():
            document = json.loads(path.read_text())
            metadata = document['metadata']
            if document['status'] != 'complete' or metadata['dirty']:
                raise ValueError('live badges require validated clean benchmark results')
            values = metrics(workload, document)
            provenance = '; ' + metadata['cpu'] + '; revision ' + metadata['revision']
        for name in names:
            (directory / (host + '-' + name + '.svg')).write_text(
                svg(workload + ' ' + name + provenance, caption, values.get(name)))


def public_badges(target, refresh_host=None):
    """Refresh both references from published public data; retain self-build SVGs."""
    for host, caption in HOSTS.items():
        result = target / host / 'results.json'
        rows, provenance = None, ''
        if result.is_file():
            document = json.loads(result.read_text())
            rows = document['programs']
            path = target / host / 'provenance.json'
            if path.is_file():
                data = json.loads(path.read_text())
                provenance = '; ' + data['cpu'] + '; revision ' + data['revision']
        directory = target / host
        directory.mkdir(exist_ok=True)
        for level in ('O0', 'O3'):
            for phase, metric in [('compile', 'compile_cpu_'), ('generated', 'run_cpu_')]:
                for reference in ('gcc', 'clang'):
                    suffix = '-clang' if reference == 'clang' else ''
                    name = level.lower() + '-' + phase + suffix
                    if (directory / (name + '.svg')).exists() and host != refresh_host:
                        continue
                    if rows is None:
                        content = svg(name, caption, None)
                    else:
                        value = public_benchmarks.speed_badge(name,
                            public_benchmarks.mean_ratio(rows, metric + level, reference)[0], 2,
                            sum(row['status'] != 'ok' for row in rows))
                        value.update(left_label=caption, label='Public ' + level + ' ' + phase + ' vs ' + reference + provenance)
                        content = badge_svg(value)
                    (directory / (name + '.svg')).write_text(content)
        for name in ('self-built', 'self-compile', 'self-built-clang', 'self-compile-clang'):
            path = directory / (name + '.svg')
            if not path.exists():
                path.write_text(svg(name, caption, None))
    benchmark_hosts.library_index(target, 'Public benchmark hosts', metadata_name='provenance.json',
                                  index_name='public-hosts.md')
    benchmark_hosts.library_index(target, 'Compiler build and codegen hosts',
                                  metadata_name='self-build-provenance.json',
                                  report_name='self-build.md', index_name='self-build-hosts.md')

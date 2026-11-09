#!/usr/bin/env python3
"""Measure the existing pinned Boost test selection with cppgm, GCC and Clang."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import tempfile

import benchmark_hosts
import benchmark_timing as timing
import boost_ci
import boost_runtime
import index_cpp_ci as common
import public_benchmarks
from publish_benchmark_badges import badge_svg

ROOT = Path(__file__).resolve().parents[1]
LABELS = ('cppgm', 'gcc', 'clang')
DEBUG_SOURCE_SHA256 = 'e0601f955d00e80e4389065030f177afca7dff2fdcf315b04649ddb759da6095'
ENUM_SOURCE_SHA256 = '9e4de657c91359e77c545adf992ec0d2e53896c9b6c3b57e1d1638262edd6e34'
GDB_JAM_SHA256 = 'ce72e499932fd3f62c1316aed0a19260ba7c13ccd1f8aa2b73996adc8bcc7923'
TRAITS_SOURCE_SHA256 = 'ee51411a31bf17b27b31de4954e949e9d6e27f40f709f28d4df3085d90323923'


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def cases(report):
    return {row['target']: row for suite in report['suites'] for row in suite['tests']}


def compile_cases(document, label):
    rows = list(cases(document['compilers'][label]).values())
    return [r for r in rows if r['target'] != boost_runtime.GDB] if optimized(document) else rows


def optimized(document):
    return 'optimization=speed' in document['metadata']['flags']


def debug_anchors(source):
    # A comment after a declaration has no machine address. Clang can resolve
    # its breakpoint outside the lexical scope. Supply a real statement there,
    # retaining every original debugger expression and expected value.
    path = source / 'libs/json/test/printers.cpp'
    original = path.read_text()
    if timing.sha(path) != DEBUG_SOURCE_SHA256:
        raise ValueError('unexpected Boost debugger source')
    marker = 'boost_benchmark_debug_anchor = 0;\n'
    text = 'static volatile unsigned boost_benchmark_debug_anchor;\n'
    for line in original.splitlines(keepends=True):
        text += line
        if '// TEST_EXPR' in line:
            text += line[:len(line) - len(line.lstrip())] + marker
    path.write_text(text)
    return dict(path='libs/json/test/printers.cpp', original_sha256=DEBUG_SOURCE_SHA256,
                adapted_sha256=timing.sha(path),
                method='Volatile breakpoint anchor after each original TEST_EXPR comment; oracles unchanged')


def debug_build_settings(source):
    path = source / 'libs/json/test/Jamfile'
    if timing.sha(path) != GDB_JAM_SHA256:
        raise ValueError('unexpected Boost debugger build settings')
    marker = '<dependency>../src/boost_json_gdb_printers.py'
    text = path.read_text()
    if text.count(marker) != 1:
        raise ValueError('missing Boost debugger target')
    path.write_text(text.replace(marker, marker + '\n      <inlining>off'))
    return dict(path='libs/json/test/Jamfile', original_sha256=GDB_JAM_SHA256,
                adapted_sha256=timing.sha(path),
                method="Retain the debugger target's original no-inline debug configuration; other targets use O3")


def enum_underlying_type(source):
    path = source / 'libs/mpl/test/issue_69.cpp'
    if timing.sha(path) != ENUM_SOURCE_SHA256:
        raise ValueError('unexpected Boost enum source')
    path.write_text(path.read_text().replace('enum test {', 'enum test : int {'))
    return dict(path='libs/mpl/test/issue_69.cpp', original_sha256=ENUM_SOURCE_SHA256,
                adapted_sha256=timing.sha(path),
                method='Fixed int underlying type makes the test enum support its instantiated -1 predecessor')


def traits_enum_underlying_type(source):
    path = source / 'libs/type_traits/test/test.hpp'
    original = timing.sha(path)
    if original != TRAITS_SOURCE_SHA256:
        raise ValueError('unexpected Boost type-traits source')
    path.write_text(path.read_text().replace('enum enum_UDT{', 'enum enum_UDT : unsigned {'))
    return dict(path='libs/type_traits/test/test.hpp', original_sha256=original,
                adapted_sha256=timing.sha(path),
                method='Fixed unsigned enum type makes TypeTraits signedness probes valid constant expressions')


def validate(document):
    if document.get('status') != 'complete' or document.get('archive_sha256') != boost_ci.ARCHIVE_SHA256:
        raise ValueError('incomplete or unpinned Boost campaign')
    if set(document['compilers']) != set(LABELS):
        raise ValueError('Boost requires all three compilers')
    expected = None
    for label, report in document['compilers'].items():
        if (report['status'] != 'passed' or len(report['suites']) != len(boost_ci.SUITES)
                or {s['project']: s['passed'] for s in report['suites']} != boost_ci.SUITES):
            raise ValueError('missing Boost suites or failed declarations: ' + label)
        selected = cases(report)
        if len(selected) != sum(boost_ci.SUITES.values()) or any(s['passed'] != len(s['tests']) for s in report['suites']):
            raise ValueError('duplicate or missing Boost declarations')
        identity = {key: row['kind'] for key, row in selected.items()}
        if expected is None:
            expected = identity
        if identity != expected:
            raise ValueError('Boost compilers did not execute the same declarations')
        for target, row in selected.items():
            value = row.get('compile_cpu', 0)
            if type(value) not in (float, int) or not math.isfinite(value) or value <= 0:
                raise ValueError('missing compiler CPU measurement: ' + label + '/' + target)
            if row.get('compiler_invocations', 0) < 1:
                raise ValueError('missing C++ compilation: ' + target)
            if row['kind'] in ('RUN', 'RUN_FAIL'):
                value = row.get('test_elapsed', 0)
                if type(value) not in (float, int) or not math.isfinite(value) or value <= 0:
                    raise ValueError('missing native test measurement: ' + label + '/' + target)
                cpu = row.get('test_cpu')
                if type(cpu) not in (float, int) or not math.isfinite(cpu) or cpu < 0:
                    raise ValueError('invalid native CPU measurement: ' + label + '/' + target)
                status = row.get('native_status')
                if type(status) is not int or not 0 <= status < 128 or (status == 0) != (row['kind'] == 'RUN'):
                    raise ValueError('incorrect native test outcome: ' + label + '/' + target)
    if 'runtime' in document:
        boost_runtime.validate(document['runtime'], list(cases(document['compilers']['cppgm']).values()))


def report(document, suite_links=True):
    text = '# Boost compilation and test execution\n\n'
    if document['status'] != 'complete':
        return text + '**Incomplete:** ' + document.get('error', document['status']) + '\n'
    text += ('Boost 1.92.0, 17 C++11 suites (1,818 declarations). '
             'Ratios are reference / cppgm, higher is better.\n\n'
             'Compilation counts child CPU of C++ test compilations, including expected rejections. '
             'It excludes configuration probes, library dependencies, C, linking and B2. Test execution includes process startup '
             'and the GDB integration test; it is correctness-test timing, not an application benchmark. '
             'The debugger source has identical volatile breakpoint anchors for all compilers; '
             'MPL and TypeTraits test enums use fixed underlying types for their -1 probes. '
             'Original test oracles are retained. '
             'Compile timings use one clean campaign per compiler; small differences need repeats.\n\n'
             '| Suite | Declarations | Compile CPU cppgm | GCC | Clang | Test CPU cppgm | GCC | Clang |\n'
             '|---|---:|---:|---:|---:|---:|---:|---:|\n')
    for project in boost_ci.SUITES:
        suites = {label: next(s for s in document['compilers'][label]['suites'] if s['project'] == project)
                  for label in LABELS}
        def total(label, key):
            return sum(row.get(key, 0) for row in suites[label]['tests']
                       if key != 'compile_cpu' or not optimized(document) or row['target'] != boost_runtime.GDB)
        compile_cpu, test_cpu = total('cppgm', 'compile_cpu'), total('cppgm', 'test_cpu')
        compile_ratios = [f"{total(label, 'compile_cpu') / compile_cpu:.3f}×" for label in ('gcc', 'clang')]
        test_ratios = [f"{total(label, 'test_cpu') / test_cpu:.3f}×" if test_cpu > 0 else 'n/a'
                       for label in ('gcc', 'clang')]
        page = 'suites/' + project.replace('/', '-') + '.md'
        name = f'[{project}]({page})' if suite_links else project
        text += (f"| {name} | {suites['cppgm']['passed']} | {compile_cpu:.3f}s | " +
                 ' | '.join(compile_ratios) + f' | {test_cpu:.3f}s | ' + ' | '.join(test_ratios) + ' |\n')
    if optimized(document):
        text += '\nPerformance uses O3 with assertions enabled; the GDB test remains Og and is correctness-only.\n'
    text += runtime_report(document, links=suite_links)
    return text + '\nExact commands, per-test times, native outcomes and toolchain/host identities are retained with the results.\n'


def runtime_report(document, project=None, links=True):
    runtime = document.get('runtime')
    if runtime is None:
        return '\nNative runtime replay was not measured in this campaign.\n'
    group = runtime if project is None else runtime['suites'].get(project)
    if group is None:
        return ''
    text = ('\n## Native runtime replay\n\n'
            'Child CPU of successful native tests, with process startup included; GDB and expected-failure '
            'executions are correctness-only. Two warmups per compiler, four A/A observations per reference '
            'and mirrored ABBA/BAAB blocks. Short tests repeat up to ten times, identically across compilers; '
            'CPU is divided by repetitions before summing tests. Executable hashes and output checks stay '
            'outside process timers. All runs must pass their original native oracle. Output hashes record '
            'equality or variation across the complete campaign; test reports may print times or addresses.\n\n'
            '| Reference | CPU speedup ↑ | Paired blocks | A/A speedup |\n'
            '|---|---:|---|---:|\n')
    for reference, value in group['comparisons'].items():
        marker = '†' if boost_runtime.noisy(value) else ''
        pairs = ', '.join(f"{p['speedup']:.3f}×" for p in value['blocks'])
        text += (f"| {reference} | {value['speedup']:.3f}×{marker} | {pairs} | "
                 f"{value['calibration']['speedup']:.3f}× |\n")
    text += ('\n† A/A drift or paired-block spread exceeds 5%; small differences are unresolved. '
             f"{runtime['selected']} native tests in the full cohort; {runtime['output_equality_cases']} "
             'have byte-identical output throughout. [Manifest](runtime/manifest.json), [paired observations](runtime/batches.jsonl) '
             'and [per-test summaries](runtime/summary.json).\n')
    if not links:
        text = text[:text.index(' [Manifest]')] + '\n'
    if project is not None:
        text = text.replace('(runtime/', '(../runtime/')
        text += ('\n| Test | Repeats | CPU cppgm | GCC | Clang | Output check |\n'
                 '|---|---:|---:|---:|---:|---|\n')
        for row in runtime['cases']:
            if row['target'].split('//')[0] == project:
                values = ' | '.join(f"{row['seconds'][label]:.6f}s" for label in LABELS)
                text += (f"| {row['target'].split('//')[1]} | {row['repeats']} | {values} | "
                         f"{row['output_check']} |\n")
    return text


def suite_reports(document, directory):
    destination = directory / 'suites'
    destination.mkdir(exist_ok=True)
    for project in boost_ci.SUITES:
        rows = {label: {r['target']: r for s in document['compilers'][label]['suites']
                        if s['project'] == project for r in s['tests']} for label in LABELS}
        name = project.replace('/', '-')
        text = (f'# {project}\n\nAll original oracles passed for all three compilers. '
                'Seconds are child CPU, one clean campaign per compiler. '
                'Test execution includes startup and GDB; small timings need repeats.\n\n'
                '| Test | Kind | Compile cppgm | GCC | Clang | Test cppgm | GCC | Clang |\n'
                '|---|---|---:|---:|---:|---:|---:|---:|\n')
        for target, row in rows['cppgm'].items():
            times = [f"{rows[label][target]['compile_cpu']:.6f}" for label in LABELS]
            times += [f"{rows[label][target]['test_cpu']:.6f}" if row['kind'] in ('RUN', 'RUN_FAIL')
                      else '—' for label in LABELS]
            text += '| ' + target.split('//', 1)[1].replace('|', '\\|') + ' | ' + row['kind'] + ' | ' + ' | '.join(times) + ' |\n'
        text += runtime_report(document, project)
        text += '\nCompiler command and timing observations: ' + ' · '.join(
            f'[{label}](../{label}/{name}/invocations.jsonl)' for label in LABELS) + '.\n\n'
        text += '[Reviewed outcomes and per-test records](../results.json) · [Host and toolchains](../metadata.json)\n'
        (destination / (name + '.md')).write_text(text)


def run(args):
    directory = args.output.resolve()
    directory.mkdir(parents=True)
    document = dict(status='running', archive_sha256=boost_ci.ARCHIVE_SHA256, compilers={})
    try:
        if timing.sha(args.archive) != boost_ci.ARCHIVE_SHA256:
            raise ValueError('Boost archive checksum mismatch')
        # Extract the verified archive ourselves; a reused source tree could have
        # edits that its archive hash cannot establish.
        with tarfile.open(args.archive) as archive:
            archive.extractall(directory / 'source', filter='data')
        source = directory / 'source/boost_1_92_0'
        adaptations = [debug_anchors(source), debug_build_settings(source),
                       enum_underlying_type(source), traits_enum_underlying_type(source)]
        with (directory / 'bootstrap.log').open('w') as log:
            subprocess.run(['./bootstrap.sh', '--with-toolset=gcc'], cwd=source,
                           stdout=log, stderr=subprocess.STDOUT, check=True)
        compilers = {label: common.compiler_path(value) for label, value in
                     [('cppgm', args.compiler), ('gcc', args.gcc), ('clang', args.clang)]}
        hashes = {label: timing.sha(path) for label, path in compilers.items()}
        library = Path(subprocess.check_output([str(compilers['gcc']), '-print-file-name=libstdc++.so'],
                                              text=True).strip()).resolve(strict=True)
        metadata = dict(revision=subprocess.check_output(['git', '-C', ROOT, 'rev-parse', 'HEAD'], text=True).strip(),
                        dirty=bool(subprocess.check_output(['git', '-C', ROOT, 'status', '--porcelain'])),
                        cpu=public_benchmarks.cpu_model(), cpus=sorted(os.sched_getaffinity(0)), jobs=args.jobs,
                        platform=platform.platform(), os_release=Path('/etc/os-release').read_text(),
                        standard_library=f'libstdc++ {library}; sha256={timing.sha(library)}',
                        compiler_hashes=hashes, archive_sha256=boost_ci.ARCHIVE_SHA256,
                        toolchains={label: subprocess.check_output([str(path), '--version'], text=True).strip()
                                    for label, path in compilers.items()},
                        tools=dict(gdb=subprocess.check_output(['gdb', '--version'], text=True).splitlines()[0],
                                   python=sys.version),
                        source_adaptations=adaptations,
                        flags=['cxxstd=11', 'variant=debug', 'optimization=speed', 'inlining=full', 'debug-symbols=off', 'link=static',
                               'runtime-link=shared', 'threading=multi'],
                        suites=boost_ci.SUITES)
        document['metadata'] = metadata
        save(directory / 'metadata.json', metadata)
        for label in LABELS:
            command = [sys.executable, str(ROOT / 'scripts/boost_ci.py'), '--boost-root', str(source),
                       '--archive', str(args.archive.resolve()), '--compiler', str(compilers[label]),
                       '--label', label, '--optimization', 'O3', '--host-cxx', str(compilers['clang' if label == 'clang' else 'gcc']),
                       '--jobs', str(args.jobs), '--output', str(directory / label)]
            with (directory / (label + '.log')).open('w') as log:
                result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
            path = directory / label / 'results.json'
            if path.is_file():
                document['compilers'][label] = json.loads(path.read_text())
            save(directory / 'results.json', document)
            if result.returncode != 0:
                raise ValueError('Boost campaign failed: ' + label + '; see ' + str(directory / (label + '.log')))
        if hashes != {label: timing.sha(path) for label, path in compilers.items()}:
            raise ValueError('a measured compiler changed')
        document['runtime'] = boost_runtime.campaign(document, directory / 'runtime', args.jobs,
                                                    blocks=args.runtime_blocks)
        document['status'] = 'complete'
        validate(document)
        suite_reports(document, directory)
    except BaseException as error:
        document.update(status='failed', error=repr(error))
        raise
    finally:
        save(directory / 'results.json', document)
        (directory / 'details.md').write_text(report(document))
        (directory / 'summary.md').write_text(report(document, suite_links=False) +
            '\nPer-test tables (`suites/`) and raw observations are in this run\'s Boost artifact.\n')


def publish(args):
    document = json.loads((args.artifact / 'results.json').read_text())
    validate(document)
    metadata = document['metadata']
    if document.get('runtime'):
        boost_runtime.validate_evidence(document['runtime'], args.artifact / 'runtime')
    if metadata['dirty'] or metadata['revision'] != args.revision or metadata.get('validation_only'):
        raise ValueError('publication requires the clean measured revision')
    if json.loads((args.artifact / 'metadata.json').read_text()) != metadata:
        raise ValueError('Boost provenance differs from the measured results')
    import boost_results
    host = benchmark_hosts.hosted_platform(metadata['cpu'])
    relative = 'boost/o3' if optimized(document) else 'boost'
    directory = args.target / relative
    history = 'history/' + args.revision + '/' + host
    base = 'https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/' + relative + '/' + history
    with tempfile.TemporaryDirectory() as temp:
        standard = Path(temp) / 'standard'
        boost_results.generate(args.artifact, standard, 'GitHub hosted; Ubuntu 26.04 container',
                               args.run_url, base + '/report.md', base + '/results.json')
        for dest in (directory / host, directory / history):
            dest.mkdir(parents=True, exist_ok=True)
            for path in retained_files(args.artifact):
                relative = path.relative_to(args.artifact)
                target = dest / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, target)
            shutil.copytree(standard, dest / 'standard', dirs_exist_ok=True)
            suite_reports(document, dest)
            (dest / 'report.md').write_text(report(document) + '\n[Shared comparison](standard/summary.md) · '
                                          '[Version-1 records](standard/records.json) · [CI run](' + args.run_url + ')\n')
    benchmark_hosts.library_index(directory, 'Boost O3 benchmark hosts' if optimized(document) else 'Boost benchmark hosts')
    (directory / 'tests.svg').write_text(badge_svg(dict(
        label='Boost tests; all three compilers', message=f'{sum(boost_ci.SUITES.values()):,} / {sum(boost_ci.SUITES.values()):,}',
        color='166534')))
    # Reuse the compact rounded badges of the other performance lanes.
    labels = {'epyc': 'EPYC', 'intel': 'Intel', 'github-intel': 'CI Intel'}
    for platform, caption in labels.items():
        for phase in ('compile', 'runtime'):
            for reference in ('gcc', 'clang'):
                path = directory / (platform + '-' + phase + '-' + reference + '.svg')
                value = None
                if platform == host and phase == 'compile':
                    candidate = sum(row['compile_cpu'] for row in compile_cases(document, 'cppgm'))
                    native = sum(row['compile_cpu'] for row in compile_cases(document, reference))
                    value = native / candidate
                elif platform == host:
                    if document.get('runtime'):
                        value = document['runtime']['comparisons'][reference]['speedup']
                elif path.exists():
                    continue
                data = dict(label='Boost ' + phase + ' CPU vs ' + reference, left_label=caption,
                            message=f'{value:.2f}×' if value is not None else 'n/m',
                            color=public_benchmarks.badge_color(value) if value is not None else '475569')
                path.write_text(badge_svg(data))


def retained_files(directory):
    for path in directory.rglob('*'):
        relative = path.relative_to(directory)
        if (not {'build', 'source'}.intersection(relative.parts) and path.is_file()
                and (path.suffix in ('.json', '.jsonl', '.xml', '.log', '.jam', '.hpp', '.md')
                     or ('runtime' in relative.parts and path.name in ('stdout', 'stderr')))):
            yield path


def collect(args):
    args.target.mkdir(parents=True, exist_ok=True)
    for path in retained_files(args.artifact):
        target = args.target / path.relative_to(args.artifact)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--archive', type=Path, required=True)
    runner.add_argument('--output', type=Path, required=True)
    runner.add_argument('--compiler', default=ROOT / 'dev/cppgm++')
    runner.add_argument('--gcc', default='g++')
    runner.add_argument('--clang', default='clang++')
    runner.add_argument('--jobs', type=int, default=len(os.sched_getaffinity(0)))
    runner.add_argument('--runtime-blocks', type=int, default=2, help='paired ABBA/BAAB blocks per reference')
    publisher = commands.add_parser('publish')
    publisher.add_argument('artifact', type=Path)
    publisher.add_argument('target', type=Path)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    collector = commands.add_parser('collect')
    collector.add_argument('artifact', type=Path)
    collector.add_argument('target', type=Path)
    args = parser.parse_args()
    if args.command == 'run':
        if args.jobs < 1 or args.runtime_blocks < 2:
            parser.error('--jobs must be positive and --runtime-blocks at least 2')
        run(args)
    elif args.command == 'publish':
        publish(args)
    else:
        collect(args)


if __name__ == '__main__':
    main()

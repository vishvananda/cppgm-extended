#!/usr/bin/env python3
"""Measure the fixed qualified course-source object cohort on all three compilers."""
import argparse
from concurrent.futures import ProcessPoolExecutor
import gzip
import hashlib
import json
import math
import os
from pathlib import Path
import resource
import shutil
import subprocess
import tarfile
import time
import urllib.request

import benchmark_hosts
from library_benchmark_badges import HOSTS, svg
import public_benchmarks
from upstream_clang_language import elf_object
from upstream_dg_proxy import execute

ROOT = Path(__file__).resolve().parents[1]
COHORT = ROOT / 'benchmarks/internal/cohort.json'
SOURCE = '783dbfcf01360d923ff590d2048a0c1764761cc3'
ARCHIVE_SHA256 = '900f16f0327ba50948f7ed06f752d42531f85e7bb46cc288247ea1d9da4c852f'
ROLES = ('cppgm', 'gcc', 'clang')


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def fetch_inputs(cache, destination):
    cache.mkdir(parents=True, exist_ok=True)
    archive = cache / (SOURCE + '.tar.gz')
    if not archive.exists():
        temporary = archive.with_suffix('.download')
        url = 'https://codeload.github.com/vishvananda/cppgm-extended/tar.gz/' + SOURCE
        with urllib.request.urlopen(url, timeout=120) as source, temporary.open('wb') as out:
            shutil.copyfileobj(source, out)
        if digest(temporary) != ARCHIVE_SHA256:
            raise ValueError('internal input archive checksum mismatch')
        temporary.replace(archive)
    if digest(archive) != ARCHIVE_SHA256:
        raise ValueError('internal input archive checksum mismatch')
    # Extract fresh from the verified archive, including companion headers.
    # Never trust a mutable cache tree or extract archive paths verbatim.
    with tarfile.open(archive) as bundle:
        for member in bundle:
            path = Path(member.name.partition('/')[2])
            if not member.isfile() or len(path.parts) < 3 or path.parts[1] != 'tests':
                continue
            if path.is_absolute() or '..' in path.parts:
                raise ValueError('unsafe internal input archive path')
            if not path.parts[0].startswith('pa') or not path.parts[0][2:].isdigit():
                continue
            target = destination / path
            target.parent.mkdir(parents=True, exist_ok=True)
            with bundle.extractfile(member) as source, target.open('wb') as out:
                shutil.copyfileobj(source, out)


def compile_case(arguments):
    case, compiler, flags, inputs, directory, limits = arguments
    directory = Path(directory) / case['id']
    directory.mkdir()
    stages = []
    for index, unit in enumerate(case['units']):
        source = Path(inputs) / unit['path']
        if digest(source) != unit['sha256']:
            raise ValueError('frozen source checksum mismatch: ' + unit['path'])
        obj = directory / f'{index}.o'
        command = [compiler, *flags, '-std=' + case['dialect'], '-O3', *case['flags'],
                   '-x', 'c++', '-c', str(source), '-o', str(obj)]
        before = resource.getrusage(resource.RUSAGE_CHILDREN)
        result, _, _ = execute(command, directory / str(index), limits)
        after = resource.getrusage(resource.RUSAGE_CHILDREN)
        result.update(unit=unit['path'], source_sha256=unit['sha256'],
                      cpu_seconds=after.ru_utime + after.ru_stime - before.ru_utime - before.ru_stime,
                      accepted=result['classification'] == 'success' and obj.is_file() and elf_object(obj))
        if obj.exists():
            result.update(object_sha256=digest(obj), object_bytes=obj.stat().st_size)
            obj.unlink()
        stages.append(result)
    return dict(id=case['id'], path=case['path'], stages=stages,
                accepted=all(stage['accepted'] for stage in stages))


def validate(document):
    metadata = document['metadata']
    if document['status'] != 'complete' or metadata['dirty'] or metadata['input_revision'] != SOURCE:
        raise ValueError('internal publication requires complete, clean, frozen-input results')
    if set(document['compilation']) != set(ROLES):
        raise ValueError('missing internal reference compiler')
    for row in document['compilation'].values():
        if (row['groups'] != 5614 or row['units'] != 5724 or row['accepted'] != 5724
                or not 0 < row['cpu_seconds'] < float('inf')):
            raise ValueError('incomplete internal compilation cohort')


def verify_observations(artifact, document, cohort=None):
    cohort = cohort or json.loads(COHORT.read_text())
    expected = {case['id']: case for case in cohort['cases']}
    for role in ROLES:
        seen, cpu, units = set(), 0.0, 0
        for line in (artifact / role / 'observations.jsonl').read_text().splitlines():
            row = json.loads(line)
            case = expected.get(row['id'])
            if case is None or row['id'] in seen or row['path'] != case['path'] or not row['accepted']:
                raise ValueError('internal observation cohort mismatch')
            seen.add(row['id'])
            if len(row['stages']) != len(case['units']):
                raise ValueError('internal observation lost a source unit')
            for stage, source in zip(row['stages'], case['units']):
                if (stage['unit'] != source['path'] or stage['source_sha256'] != source['sha256']
                        or not stage['accepted'] or stage['returncode'] != 0
                        or stage['classification'] != 'success' or len(stage['object_sha256']) != 64
                        or stage['object_bytes'] <= 0 or not 0 < stage['cpu_seconds'] < float('inf')):
                    raise ValueError('invalid internal compilation observation')
                cpu += stage['cpu_seconds']
                units += 1
        summary = document['compilation'][role]
        if (seen != set(expected) or summary['groups'] != len(seen)
                or summary['units'] != units or summary['accepted'] != units
                or not math.isclose(cpu, summary['cpu_seconds'], rel_tol=1e-12)):
            raise ValueError('internal observations disagree with summary')


def report(document):
    metadata = document['metadata']
    lines = ['# Internal C++ source compilation', '',
             'Fixed qualified cohort: **5,614 fixture groups / 5,724 translation units**, at `-O3`.', '',
             'This measures object compilation of the common positive source subset, including driver startup. '
             'It does not replace the original assignment IR, ABI, diagnostic or runtime oracles. '
             'The fixed source selection is independent of timing; any failed compilation fails the run.', '',
             f"Measured compiler: `{metadata['revision']}`. Input snapshot: `{SOURCE}`.", '',
             f"CPU: {metadata['cpu']}. Workers per compiler: {metadata['jobs']}.", '',
             '| Compiler | Accepted objects | Compile CPU seconds | Speed vs cppgm ↑ |',
             '|---|---:|---:|---:|']
    candidate = document['compilation'].get('cppgm', {}).get('cpu_seconds', 0)
    for role, row in document['compilation'].items():
        ratio = f"{row['cpu_seconds'] / candidate:.2f}×" if candidate else 'n/m'
        lines.append(f"| {role} | {row['accepted']} / {row['units']} | {row['cpu_seconds']:.3f} | {ratio} |")
    lines += ['', 'One complete observation per compiler; sums include compiler subprocesses. '
              'Linking, execution and harness time are excluded. This is a compile-throughput observation, '
              'with no application-runtime score or claim about small changes.', '',
              '[Host and compiler identities](metadata.json) · [Measured results](results.json)']
    return '\n'.join(lines) + '\n'


def run(args):
    directory = args.output.resolve()
    directory.mkdir(parents=True, exist_ok=False)
    cohort = json.loads(COHORT.read_text())
    if cohort['source_revision'] != SOURCE or len(cohort['cases']) != 5614:
        raise ValueError('unexpected internal source cohort')
    compilers = {role: str(Path(shutil.which(str(getattr(args, role))) or getattr(args, role)).absolute()) for role in ROLES}
    metadata = dict(**benchmark_hosts.source_metadata(args.repo_root),
                    cpu=public_benchmarks.cpu_model(), jobs=args.jobs, input_revision=SOURCE,
                    cohort_sha256=digest(COHORT), archive_sha256=ARCHIVE_SHA256,
                    compiler_hashes={role: digest(binary) for role, binary in compilers.items()},
                    toolchains={role: subprocess.check_output([binary, '--version'], text=True).splitlines()[0]
                                for role, binary in compilers.items()}, resources=dict(cohort['resources'], timeout_seconds=60),
                    cpus=sorted(os.sched_getaffinity(0)))
    document = dict(status='running', metadata=metadata, compilation={})
    save(directory / 'metadata.json', metadata)
    os.environ['CPPGM_HOST_CXX'] = compilers['gcc']
    stack = cohort['resources']['stack_bytes']
    resource.setrlimit(resource.RLIMIT_STACK, (stack, resource.getrlimit(resource.RLIMIT_STACK)[1]))
    limits = dict(cohort['resources'], timeout_seconds=60, compiler_error_scan=True)
    gcc_install = str(Path(subprocess.check_output([compilers['gcc'], '-print-libgcc-file-name'], text=True).strip()).parent)
    try:
        fetch_inputs(args.cache, directory / 'source')
        for role, compiler in compilers.items():
            start = time.monotonic()
            output = directory / role
            output.mkdir()
            flags = ['--gcc-install-dir=' + gcc_install] if role == 'clang' else []
            rows = []
            with (output / 'observations.jsonl').open('w') as observations, ProcessPoolExecutor(args.jobs) as pool:
                work = [(case, compiler, flags, str(directory / 'source'), str(output), limits) for case in cohort['cases']]
                for row in pool.map(compile_case, work, chunksize=4):
                    observations.write(json.dumps(row) + '\n')
                    rows.append(row)
            stages = [stage for row in rows for stage in row['stages']]
            summary = dict(groups=len(rows), units=len(stages), accepted=sum(s['accepted'] for s in stages),
                           cpu_seconds=sum(s['cpu_seconds'] for s in stages), elapsed_seconds=time.monotonic() - start)
            document['compilation'][role] = summary
            save(directory / 'results.json', document)
            print(role, summary, flush=True)
            failed = [row['path'] for row in rows if not row['accepted']]
            if failed:
                raise ValueError('internal compilation failures: ' + ', '.join(failed[:10]))
        if metadata['compiler_hashes'] != {role: digest(binary) for role, binary in compilers.items()}:
            raise ValueError('compiler changed during internal measurement')
        document['status'] = 'complete'
    except Exception as error:
        document.update(status='failed', error=str(error))
        raise
    finally:
        save(directory / 'results.json', document)
        (directory / 'details.md').write_text(report(document))


def badges(target):
    for host, caption in HOSTS.items():
        path = target / host / 'results.json'
        document = json.loads(path.read_text()) if path.exists() else None
        if document:
            validate(document)
        for reference in ('gcc', 'clang'):
            value, provenance = None, ''
            if document:
                data = document['metadata']
                provenance = f"; {data['cpu']}; revision {data['revision']}"
                values = document['compilation']
                value = values[reference]['cpu_seconds'] / values['cppgm']['cpu_seconds']
            (target / (host + '-compile-' + reference + '.svg')).write_text(
                svg('Internal C++ O3 compile vs ' + reference + provenance, caption, value))
    benchmark_hosts.library_index(target, 'Internal C++ benchmark hosts')


def publish(args):
    document = json.loads((args.artifact / 'results.json').read_text())
    validate(document)
    metadata = document['metadata']
    if metadata['revision'] != args.revision or document.get('historical'):
        raise ValueError('internal artifact does not match the measured revision')
    if json.loads((args.artifact / 'metadata.json').read_text()) != metadata:
        raise ValueError('internal artifact metadata differs')
    if metadata['cohort_sha256'] != digest(COHORT) or metadata['archive_sha256'] != ARCHIVE_SHA256:
        raise ValueError('internal artifact input identity differs')
    verify_observations(args.artifact, document)
    host = benchmark_hosts.publication_platform(metadata['cpu'], args.platform)
    root = args.target / 'internal'
    for destination in [root / host, root / 'history' / args.revision / host]:
        destination.mkdir(parents=True, exist_ok=True)
        for name in ('results.json', 'metadata.json'):
            shutil.copyfile(args.artifact / name, destination / name)
        for role in ROLES:
            source = args.artifact / role / 'observations.jsonl'
            (destination / (role + '-observations.jsonl.gz')).write_bytes(gzip.compress(source.read_bytes(), mtime=0))
        from internal_cpp_results import write
        url = 'https://github.com/vishvananda/cppgm-extended/blob/benchmark-badges/' + str(destination.relative_to(args.target))
        write(document, destination, benchmark_hosts.environment(host), args.run_url, url)
        links = ' · '.join(f'[{role} per-unit records]({role}-observations.jsonl.gz)' for role in ROLES)
        (destination / 'report.md').write_text(report(document) + f'\n[CI run]({args.run_url}) · '
            '[Shared comparison records](records.json)\n\n' + links + '\n')
    badges(root)


def seed(target):
    root = target / 'internal'
    destination = root / 'intel'
    if not (destination / 'results.json').exists():
        document = json.loads((ROOT / 'benchmarks/internal/baseline.json').read_text())
        validate(document)
        destination.mkdir(parents=True, exist_ok=True)
        save(destination / 'results.json', document)
        save(destination / 'metadata.json', document['metadata'])
        (destination / 'report.md').write_text(report(document) + '\nHistorical local snapshot; '
            '[original report](https://github.com/vishvananda/cppgm-extended/blob/main/docs/performance/preview/internal.md).\n')
    badges(root)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    runner = commands.add_parser('run')
    runner.add_argument('--output', type=Path, required=True)
    runner.add_argument('--cache', type=Path, default=ROOT / 'obj/internal-inputs')
    runner.add_argument('--repo-root', type=Path, default=ROOT)
    runner.add_argument('--compiler', dest='cppgm', default=str(ROOT / 'dev/cppgm++'))
    runner.add_argument('--gcc', default='g++')
    runner.add_argument('--clang', default='clang++')
    runner.add_argument('--jobs', type=int, default=4)
    publisher = commands.add_parser('publish')
    publisher.add_argument('artifact', type=Path)
    publisher.add_argument('target', type=Path)
    publisher.add_argument('--revision', required=True)
    publisher.add_argument('--run-url', required=True)
    publisher.add_argument('--platform', choices=('github', 'intel'), default='github')
    initializer = commands.add_parser('seed')
    initializer.add_argument('target', type=Path)
    args = parser.parse_args()
    if args.command == 'run':
        if args.jobs < 1:
            parser.error('--jobs must be positive')
        run(args)
    elif args.command == 'publish':
        publish(args)
    else:
        seed(args.target)


if __name__ == '__main__':
    main()

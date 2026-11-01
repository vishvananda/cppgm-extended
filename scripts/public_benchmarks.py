#!/usr/bin/env python3
"""Compare cppgm++ with g++ and clang++ on the pinned public benchmarks.

fetch   Clone each source at its pinned commit and apply its patch.
run     Build and run each program with each compiler at each level.
report  Write the summary and compare the counts with the baseline.

See benchmarks/public/README.md.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import zipfile


ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / 'benchmarks/public/manifest.json'
BASELINE = ROOT / 'benchmarks/public/baseline.json'
COMPILE_TIMEOUT = 900
RUN_TIMEOUT = 300
COUNT_TIMEOUT = 3600
TIERS = {'compile', 'run'}
# The tiers each compiler is counted in. A count under cachegrind costs about
# 80 times the run it measures, and g++ and clang++ compiles take most of that.
COUNTED_TIERS = {'cppgm': TIERS, 'gcc': {'run'}, 'clang': set()}
# The largest change that is not reported.
TOLERANCE = {
    'frontend_instructions': 0.005,
    'compile_instructions': 0.005,
    'run_instructions': 0.01,
}
DRIFT = 0.005
# CPU times below this are ignored.
MINIMUM_CPU = 0.01
METRICS = (
    ('frontend_cpu', 'Frontend CPU, `-fsyntax-only`'),
    ('frontend_instructions', 'Frontend instructions'),
    ('compile_cpu_O0', 'Compile CPU, `-O0`'),
    ('compile_cpu_O3', 'Compile CPU, `-O3`'),
    ('compile_instructions', 'Compile instructions, `-O3`'),
    ('compile_rss_O0', 'Compile peak RSS, `-O0`'),
    ('compile_rss_O3', 'Compile peak RSS, `-O3`'),
    ('size_O0', 'Executable size, `-O0`'),
    ('size_O3', 'Executable size, `-O3`'),
    ('run_cpu_O0', 'Run CPU, `-O0`'),
    ('run_cpu_O3', 'Run CPU, `-O3`'),
    ('run_instructions', 'Run instructions, `-O3`'),
)
# The README badges, one for each level.
BADGE_LEVELS = ('O0', 'O3')
REFERENCES = (('gcc', 'g++'), ('clang', 'clang++'))


def require(condition, message):
    if not condition:
        raise ValueError(message)


def load_manifest(path=MANIFEST):
    manifest = json.loads(Path(path).read_text())
    validate(manifest, Path(path).parent)
    return manifest


def validate(manifest, directory):
    for name, source in manifest['sources'].items():
        require(re.fullmatch('[0-9a-f]{40}', source['commit']),
                f'{name}: the commit must be a full hash')
        require(source.get('license'), f'{name}: no license recorded')
        if 'patch' in source:
            require((directory / source['patch']).is_file(),
                    f'{name}: missing {source["patch"]}')
    seen = set()
    for program in manifest['programs']:
        name = program['name']
        require(name not in seen, f'{name}: listed twice')
        seen.add(name)
        require(program['source'] in manifest['sources'],
                f'{name}: unknown source {program["source"]}')
        require(program['srcs'], f'{name}: no sources')
        require(set(program.get('count', [])) <= TIERS,
                f'{name}: unknown count tier')
        for src in program['srcs']:
            if src.startswith('@drivers/'):
                require((directory / src[1:]).is_file(), f'{name}: missing {src}')


def pin_digest(source, directory):
    state = json.dumps(source, sort_keys=True).encode()
    if 'patch' in source:
        state += (directory / source['patch']).read_bytes()
    return hashlib.sha256(state).hexdigest()


def git(target, *arguments):
    subprocess.run(['git', '-C', str(target), *arguments], check=True)


def fetch_source(name, source, directory, cache, retries=3):
    target = cache / name
    marker = cache / f'{name}.pin'
    digest = pin_digest(source, directory)
    if marker.is_file() and marker.read_text() == digest and target.is_dir():
        return False
    marker.unlink(missing_ok=True)
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)
    git(target, 'init', '-q')
    git(target, 'remote', 'add', 'origin', source['url'])
    git(target, 'sparse-checkout', 'set', '--no-cone',
        *[f'/{path}' for path in source['paths']])
    for attempt in range(retries):
        try:
            git(target, 'fetch', '-q', '--depth', '1', '--filter=blob:none',
                'origin', source['commit'])
            git(target, 'checkout', '-q', 'FETCH_HEAD')
            break
        except subprocess.CalledProcessError:
            if attempt + 1 == retries:
                raise
    head = subprocess.check_output(['git', '-C', str(target), 'rev-parse', 'HEAD'],
                                   text=True).strip()
    require(head == source['commit'], f'{name}: checked out {head}')
    for path in source.get('strip_cr', []):
        file = target / path
        file.write_bytes(file.read_bytes().replace(b'\r\n', b'\n'))
    if 'unzip' in source:
        with zipfile.ZipFile(target / source['unzip']) as archive:
            archive.extractall(target)
    if 'patch' in source:
        subprocess.run(['patch', '--binary', '-s', '-p1', '-i',
                        str((directory / source['patch']).resolve())],
                       cwd=target, check=True)
    marker.write_text(digest)
    return True


def generate_inputs(manifest, cache, gxx):
    """Generate the input files with programs built by g++."""
    output = cache / 'generated'
    output.mkdir(exist_ok=True)
    for item in manifest.get('generated', []):
        target = output / item['output']
        if target.is_file():
            continue
        cwd = cache / item['source'] / item['dir']
        with tempfile.TemporaryDirectory() as scratch:
            program = Path(scratch) / 'generate'
            subprocess.run([gxx, '-O2', '-w', '-x', 'c++', item['src'],
                            '-o', str(program), '-lm', '-pthread'],
                           cwd=cwd, check=True)
            partial = target.with_suffix('.partial')
            with partial.open('wb') as stream:
                subprocess.run([str(program), *item['args']], cwd=cwd,
                               stdout=stream, check=True)
            partial.rename(target)


def fetch(arguments):
    directory = Path(arguments.manifest).parent
    manifest = load_manifest(arguments.manifest)
    cache = Path(arguments.cache).resolve()
    cache.mkdir(parents=True, exist_ok=True)
    for name, source in manifest['sources'].items():
        fresh = fetch_source(name, source, directory, cache)
        print(f'{name}: {"fetched" if fresh else "cached"} {source["commit"][:12]}',
              flush=True)
    generate_inputs(manifest, cache, arguments.gxx)
    return 0


def measure(command, cwd, timeout, log, stdin=None, stdout=None):
    """Run command. Return its exit code, CPU seconds and peak RSS in KiB."""
    source = open(stdin, 'rb') if stdin else subprocess.DEVNULL
    with open(log, 'wb') as errors:
        process = subprocess.Popen(command, cwd=cwd, stdin=source,
                                   stdout=stdout if stdout else errors,
                                   stderr=errors, start_new_session=True)
    expired = threading.Event()

    def expire():
        expired.set()
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass

    timer = threading.Timer(timeout, expire)
    timer.start()
    try:
        _, status, usage = os.wait4(process.pid, 0)
    finally:
        timer.cancel()
        if stdin:
            source.close()
    process.returncode = os.waitstatus_to_exitcode(status)
    return dict(code=process.returncode, timed_out=expired.is_set(),
                cpu=usage.ru_utime + usage.ru_stime, rss=usage.ru_maxrss)


def instruction_count(log_text):
    """Return the instruction count in a cachegrind log, or None."""
    match = re.search(r'I\s+refs:\s+([\d,]+)', log_text)
    return int(match.group(1).replace(',', '')) if match else None


def counted(command, cwd, work, stdin=None, stdout=None):
    """Run command under cachegrind and add the counts of its processes."""
    logs = Path(work)
    if logs.exists():
        shutil.rmtree(logs)
    logs.mkdir(parents=True)
    wrapped = ['valgrind', '--tool=cachegrind', '--cache-sim=no',
               '--cachegrind-out-file=/dev/null', '--trace-children=yes',
               f'--log-file={logs}/cachegrind.%p', *command]
    result = measure(wrapped, cwd, COUNT_TIMEOUT, logs / 'stderr', stdin, stdout)
    counts = [instruction_count(path.read_text(errors='replace'))
              for path in logs.glob('cachegrind.*')]
    counts = [count for count in counts if count is not None]
    result['instructions'] = sum(counts) if counts else None
    return result


def first_error(path):
    lines = Path(path).read_text(errors='replace').splitlines()
    for line in lines:
        if re.search(r'\b(error|ERROR|fatal|undefined reference|Assertion)\b', line):
            return line[:300]
    return next((line[:300] for line in lines if line.strip()), '')


def level_key(metric, level):
    return f'{metric}_{level.lstrip("-")}'


class Side:
    """One program built by one compiler."""

    def __init__(self, context, program, label):
        self.context = context
        self.program = program
        self.label = label
        self.tool = context.tools[label]
        self.work = context.work / re.sub(r'[^A-Za-z0-9_.-]', '_', program['name']) / label
        self.cwd = context.cache / program['source'] / program.get('dir', '')
        self.facts = {}
        self.objects = []

    def flags(self, level):
        manifest = self.context.manifest
        program = self.program
        flags = [program.get('std', manifest['default_std']), level]
        flags += manifest['common_flags'] + program.get('flags', [])
        return flags + manifest['references'].get(self.label, [])

    def source(self, src):
        if src.startswith('@drivers/'):
            return str(self.context.directory / src[1:])
        return src

    def by_gcc(self, src):
        return self.program.get('c_with_gcc') and src.endswith('.c')

    def compile_command(self, src, level, arguments):
        if self.by_gcc(src):
            flags = ['-std=gnu11', level, '-w', '-fpermissive']
            return [self.context.gcc, *flags, *self.program.get('flags', []),
                    *arguments, self.source(src)]
        return [self.tool, *self.flags(level), *arguments, self.source(src)]

    def counted_level(self):
        return self.context.manifest['levels'][-1]

    def counting(self, tier, level):
        return (tier in self.program.get('count', []) and tier in COUNTED_TIERS[self.label]
                and self.context.count and level == self.counted_level())

    def step(self, mode, level):
        """Compile all sources in mode. Do not measure C files that gcc builds."""
        total = 0.0
        rss = 0
        instructions = 0
        counting = self.counting('compile', level)
        work = self.work / level
        self.objects = []
        for index, src in enumerate(self.program['srcs']):
            if mode == 'frontend':
                if self.by_gcc(src):
                    continue
                arguments = ['-fsyntax-only']
            else:
                target = work / f'{index}.o'
                self.objects.append(str(target))
                arguments = ['-c', '-o', str(target)]
            command = self.compile_command(src, level, arguments)
            log = work / f'{mode}{index}.log'
            result = measure(command, self.cwd, COMPILE_TIMEOUT, log)
            if result['code'] or result['timed_out']:
                kind = 'timed out' if result['timed_out'] else f'exit {result["code"]}'
                return f'{mode} {level} {src}: {kind}: {first_error(log)}'
            if self.by_gcc(src):
                continue
            total += result['cpu']
            rss = max(rss, result['rss'])
            if counting:
                count = counted(command, self.cwd, work / f'count-{mode}{index}')
                if count['code'] or count['instructions'] is None:
                    return f'{mode} {level} {src}: cachegrind exit {count["code"]}'
                instructions += count['instructions']
        if mode == 'frontend':
            self.facts['frontend_cpu'] = round(total, 3)
        else:
            self.facts[level_key('compile_cpu', level)] = round(total, 3)
            self.facts[level_key('compile_rss', level)] = rss
        if counting:
            self.facts[f'{mode}_instructions'] = instructions
        return None

    def link(self, level):
        manifest = self.context.manifest
        self.exe = self.work / level / 'a.out'
        log = self.work / level / 'link.log'
        command = [self.context.gxx, *self.objects, '-o', str(self.exe),
                   *self.program.get('ldflags', []), *manifest['ldflags']]
        result = measure(command, self.cwd, COMPILE_TIMEOUT, log)
        if result['code'] or result['timed_out']:
            return f'link {level}: {first_error(log)}'
        self.facts[level_key('size', level)] = self.exe.stat().st_size
        return None

    def stdin(self):
        path = self.program.get('stdin')
        if path is None:
            return None
        if path.startswith('@generated/'):
            return self.context.cache / path[1:]
        return self.cwd / path

    def run(self, level):
        command = [str(self.exe), *self.program.get('args', [])]
        work = self.work / level
        output = work / 'stdout'
        with output.open('wb') as stream:
            result = measure(command, self.cwd, RUN_TIMEOUT, work / 'run.log',
                             self.stdin(), stream)
        if result['timed_out']:
            return f'run {level}: timed out'
        self.facts[level_key('run_cpu', level)] = round(result['cpu'], 3)
        self.facts[level_key('exit_code', level)] = result['code']
        self.facts[level_key('output', level)] = hashlib.sha256(output.read_bytes()).hexdigest()
        if self.counting('run', level):
            with open(os.devnull, 'wb') as discard:
                count = counted(command, self.cwd, work / 'count-run', self.stdin(), discard)
            if count['code'] != result['code'] or count['instructions'] is None:
                return f'run {level}: cachegrind exit {count["code"]}'
            self.facts['run_instructions'] = count['instructions']
        return None

    def build_and_run(self):
        if self.work.exists():
            shutil.rmtree(self.work)
        levels = self.context.manifest['levels']
        for level in levels:
            (self.work / level).mkdir(parents=True)
        error = self.step('frontend', self.counted_level())
        for level in levels:
            error = error or self.step('compile', level) or self.link(level) or self.run(level)
        return error


def disagreement(side, reference):
    """Describe the first level where the binaries differ, or return None."""
    for level in side.context.manifest['levels']:
        keys = level_key('output', level), level_key('exit_code', level)
        if any(side.facts[key] != reference.facts[key] for key in keys):
            return (f'{level} differs from g++, exit {side.facts[keys[1]]} '
                    f'and {reference.facts[keys[1]]}')
    return None


def run_program(context, program):
    gcc = Side(context, program, 'gcc')
    cppgm = Side(context, program, 'cppgm')
    result = dict(name=program['name'], status='ok', gcc=gcc.facts, cppgm=cppgm.facts)
    if 'clang' in context.tools:
        clang = Side(context, program, 'clang')
        result['clang'] = clang.facts
    error = gcc.build_and_run()
    if error:
        result.update(status='gcc-failed', error=error)
        return result
    error = cppgm.build_and_run()
    if error:
        status = 'link-failed' if error.startswith('link') else (
            'run-failed' if error.startswith('run') else 'compile-failed')
        result.update(status=status, error=error)
    elif disagreement(cppgm, gcc):
        result.update(status='output-mismatch', error=disagreement(cppgm, gcc))
    if 'clang' in context.tools:
        error = clang.build_and_run()
        error = error or disagreement(clang, gcc)
        if error:
            result['clang_error'] = error
            clang.facts.clear()
    return result


def tool_version(command):
    try:
        output = subprocess.run(command, capture_output=True, text=True).stdout
    except OSError:
        return None
    return output.splitlines()[0] if output else None


def run(arguments):
    manifest = load_manifest(arguments.manifest)
    tools = dict(gcc=arguments.gxx, cppgm=str(Path(arguments.cppgm).resolve()))
    if not arguments.no_clang:
        tools['clang'] = arguments.clang
    context = argparse.Namespace(
        manifest=manifest, directory=Path(arguments.manifest).resolve().parent,
        cache=Path(arguments.cache).resolve(), work=Path(arguments.work).resolve(),
        gcc=arguments.gcc, gxx=arguments.gxx, tools=tools,
        count=not arguments.no_count)
    programs = manifest['programs']
    if arguments.only:
        programs = [p for p in programs if re.search(arguments.only, p['name'])]
    results = []
    with ThreadPoolExecutor(arguments.jobs) as pool:
        for result in pool.map(lambda p: run_program(context, p), programs):
            results.append(result)
            print(f'{result["name"]}: {result["status"]} {result.get("error", "")}',
                  flush=True)
    document = dict(
        environment=dict(gcc=tool_version([arguments.gxx, '--version']),
                         clang=tool_version([tools.get('clang', 'false'), '--version']),
                         valgrind=tool_version(['valgrind', '--version'])),
        programs=results)
    Path(arguments.output).write_text(json.dumps(document, indent=1) + '\n')
    return 0


def geomean(ratios):
    return math.exp(sum(math.log(ratio) for ratio in ratios) / len(ratios)) if ratios else None


def ratio(result, metric, reference='gcc'):
    before = result.get(reference, {}).get(metric)
    after = result['cppgm'].get(metric)
    if not before or not after:
        return None
    if '_cpu' in metric and min(before, after) < MINIMUM_CPU:
        return None
    return after / before


def mean_ratio(results, metric, reference):
    """Return the geometric mean over the passing programs, and their number."""
    ratios = [ratio(r, metric, reference) for r in results if r['status'] == 'ok']
    ratios = [value for value in ratios if value]
    return geomean(ratios), len(ratios)


def badge_color(slowest):
    for limit, color in ((1.0, 'brightgreen'), (0.8, 'green'), (0.67, 'yellow')):
        if slowest >= limit:
            return color
    return 'orange'


def badge(results, level):
    """Return the shields.io endpoint document for level."""
    compile_ratio = mean_ratio(results, f'compile_cpu_{level}', 'gcc')[0]
    run_ratio = mean_ratio(results, f'run_cpu_{level}', 'gcc')[0]
    failures = sum(r['status'] != 'ok' for r in results)
    if compile_ratio and run_ratio:
        compile_speed = 1 / compile_ratio
        run_speed = 1 / run_ratio
        message = f'compile speed {compile_speed:.1f}× gcc // generated {run_speed:.2f}× gcc'
        color = badge_color(min(compile_speed, run_speed))
    else:
        message = 'no data'
        color = 'lightgrey'
    if failures:
        message += f' // {failures} failing'
        color = 'red'
    return dict(schemaVersion=1, label=f'-{level}', message=message, color=color)


def compare_baseline(results, baseline):
    """Return the counts that changed by more than their tolerance."""
    changes = []
    programs = baseline.get('programs', {})
    for result in results:
        recorded = programs.get(result['name'], {})
        for metric, tolerance in TOLERANCE.items():
            before = recorded.get('cppgm', {}).get(metric)
            after = result['cppgm'].get(metric)
            if not before or not after:
                continue
            change = after / before - 1
            if abs(change) <= tolerance:
                continue
            gcc_before = recorded.get('gcc', {}).get(metric)
            gcc_after = result['gcc'].get(metric)
            drift = bool(gcc_before and gcc_after
                         and abs(gcc_after / gcc_before - 1) > DRIFT)
            changes.append(dict(name=result['name'], metric=metric, before=before,
                                after=after, change=change, drift=drift))
    return changes


def baseline_of(results):
    programs = {}
    for result in results:
        entry = {}
        for label in ('gcc', 'clang', 'cppgm'):
            facts = {metric: result[label][metric] for metric in TOLERANCE
                     if metric in result.get(label, {})}
            if facts:
                entry[label] = facts
        if result['status'] == 'ok' and entry:
            programs[result['name']] = entry
    return dict(programs=programs)


def summary(document, changes):
    results = document['programs']
    failures = [r for r in results if r['status'] != 'ok']
    lines = ['# Public benchmarks', '',
             f'{len(results) - len(failures)} of {len(results)} programs built '
             'with cppgm++ and g++ and agreed. Ratios are cppgm++ over the reference, '
             'so below 1 means cppgm++ is smaller or faster.', '',
             '| Measure | vs g++ | Programs | vs clang++ | Programs |',
             '| --- | ---: | ---: | ---: | ---: |']
    for metric, title in METRICS:
        cells = []
        for reference, _ in REFERENCES:
            mean, count = mean_ratio(results, metric, reference)
            cells += [f'{mean:.3f}' if mean else '-', str(count)]
        if cells[1] != '0' or cells[3] != '0':
            lines.append(f'| {title} | {" | ".join(cells)} |')
    if failures:
        lines += ['', '## Failures', '', '| Program | Status | Detail |',
                  '| --- | --- | --- |']
        for result in failures:
            lines.append(f'| {result["name"]} | {result["status"]} | '
                         f'`{result.get("error", "")[:160]}` |')
    declined = [r for r in results if 'clang_error' in r]
    if declined:
        lines += ['', '## Left out of the clang++ column', '', '| Program | Detail |',
                  '| --- | --- |']
        lines += [f'| {r["name"]} | `{r["clang_error"][:160]}` |' for r in declined]
    if changes:
        lines += ['', '## Changes from the baseline', '',
                  '| Program | Measure | Baseline | Now | Change |',
                  '| --- | --- | ---: | ---: | ---: |']
        for change in sorted(changes, key=lambda c: -c['change']):
            note = ' (g++ moved too)' if change['drift'] else ''
            lines.append(f'| {change["name"]} | {change["metric"]} | {change["before"]:,} | '
                         f'{change["after"]:,} | {change["change"]:+.2%}{note} |')
    worst = [(ratio(r, 'run_instructions'), r['name']) for r in results if r['status'] == 'ok']
    worst = sorted(((value, name) for value, name in worst if value), reverse=True)[:10]
    if worst:
        lines += ['', '## Largest run-time instruction ratios', '',
                  '| Program | Ratio |', '| --- | ---: |']
        lines += [f'| {name} | {value:.2f} |' for value, name in worst]
    return '\n'.join(lines) + '\n'


def report(arguments):
    return main_report(arguments.results, arguments.summary, arguments.baseline,
                       arguments.write_baseline, arguments.badges)


def main_report(results_path, summary_path=None, baseline_path=None, write_baseline=None,
                badges=None):
    document = json.loads(Path(results_path).read_text())
    results = document['programs']
    if write_baseline:
        Path(write_baseline).write_text(
            json.dumps(baseline_of(results), indent=1, sort_keys=True) + '\n')
    if badges:
        Path(badges).mkdir(parents=True, exist_ok=True)
        for level in BADGE_LEVELS:
            (Path(badges) / f'{level.lower()}.json').write_text(
                json.dumps(badge(results, level), indent=1) + '\n')
    baseline = {}
    if baseline_path and Path(baseline_path).is_file():
        baseline = json.loads(Path(baseline_path).read_text())
    changes = compare_baseline(results, baseline)
    text = summary(document, changes)
    if summary_path:
        with open(summary_path, 'a') as stream:
            stream.write(text)
    print(text)
    for change in changes:
        if change['change'] > 0 and not change['drift']:
            print(f'::warning::{change["name"]}: {change["metric"]} '
                  f'{change["change"]:+.2%} from the baseline')
    return 1 if any(r['status'] != 'ok' for r in results) else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--manifest', default=str(MANIFEST))
    commands = parser.add_subparsers(dest='command', required=True)

    command = commands.add_parser('fetch')
    command.add_argument('--cache', required=True)
    command.add_argument('--gxx', default='g++')
    command.set_defaults(handler=fetch)

    command = commands.add_parser('run')
    command.add_argument('--cache', required=True)
    command.add_argument('--work', required=True)
    command.add_argument('--output', required=True)
    command.add_argument('--cppgm', default=str(ROOT / 'dev/cppgm++'))
    command.add_argument('--gcc', default='gcc')
    command.add_argument('--gxx', default='g++')
    command.add_argument('--clang', default='clang++')
    command.add_argument('--no-clang', action='store_true',
                         help='compare with g++ alone')
    command.add_argument('--only', help='a regular expression of program names')
    command.add_argument('--no-count', action='store_true',
                         help='skip cachegrind, measuring CPU time alone')
    command.add_argument('-j', '--jobs', type=int, default=os.cpu_count())
    command.set_defaults(handler=run)

    command = commands.add_parser('report')
    command.add_argument('results')
    command.add_argument('--baseline', default=str(BASELINE))
    command.add_argument('--summary', help='a file to append the summary to')
    command.add_argument('--write-baseline', help='record these results as the baseline')
    command.add_argument('--badges', help='a directory to write the README badges to')
    command.set_defaults(handler=report)

    arguments = parser.parse_args()
    return arguments.handler(arguments)


if __name__ == '__main__':
    sys.exit(main())

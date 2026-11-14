#!/usr/bin/env python3
"""Compare complete compiler rebuilds by cppgm-built and Clang-built WASI compilers."""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import resource
import shutil
import statistics
import subprocess
import sys
import time

from experimental import wasi_inception as inception
import public_benchmarks

ROOT = Path(__file__).resolve().parents[1]


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def matched_flags(manifest, clang=False):
    # Both producers consume the same frozen headers, EH libraries, feature
    # macros and single-threaded libc++ configuration. Only Clang needs its
    # instruction-selection switches and an explicit EH header search path.
    flags = [*manifest['flags'], '-matomics', '-fno-strict-aliasing', '-Lsysroot/lib/wasm32-wasip1/eh']
    if clang:
        flags += ['-fexceptions', '-fwasm-exceptions', '-mllvm', '-wasm-use-legacy-eh=false',
                  '-isystem', 'sysroot/include/wasm32-wasip1/eh/c++/v1']
    return flags


def outputs_equal(expected, actual):
    if set(expected) != set(actual):
        raise RuntimeError('full rebuild output set changed')
    for name in expected:
        inception.compare(expected[name], actual[name])


def paired(rows, metric):
    if not rows or len(rows) % 4:
        raise ValueError('incomplete ABBA blocks')
    ratios = []
    for at in range(0, len(rows), 4):
        block = rows[at:at + 4]
        if [r['slot'] for r in block] != [0, 1, 1, 0]:
            raise ValueError('misordered ABBA block')
        a = sum(r[metric] for r in (block[0], block[3]))
        b = sum(r[metric] for r in (block[1], block[2]))
        if a <= 0 or b <= 0:
            raise ValueError('nonpositive timing')
        ratios.append(b / a)
    return dict(candidate=statistics.median(r[metric] for r in rows if r['slot'] == 0),
                reference=statistics.median(r[metric] for r in rows if r['slot'] == 1),
                speedup=statistics.median(ratios), ratios=ratios)


class Campaign:
    def __init__(self, args, package, manifest):
        self.args, self.package, self.manifest = args, package, manifest
        self.inputs = package / 'inputs'
        self.rows = []
        self.expected = None
        self.flags = matched_flags(manifest)

    def command(self, command, log):
        with log.open('wb') as stream:
            process = subprocess.Popen(command, cwd=self.inputs, stdout=stream, stderr=stream,
                                       start_new_session=True)
            try:
                status = process.wait(timeout=self.args.timeout)
            finally:
                if process.poll() is None:
                    import signal
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait()
        if status:
            raise RuntimeError(f'command failed ({status}); see {log}')

    def build(self, runner, name, flags, slot=None, check=True):
        directory = self.inputs / name
        directory.mkdir()
        commands = []

        def compile_source(source):
            output = directory / inception.object_name(source)
            command = [*runner, *flags, '-c', source, '-o', str(output.relative_to(self.inputs))]
            begin = time.monotonic()
            self.command(command, output.with_suffix('.log'))
            return dict(source=source, command=command, seconds=time.monotonic() - begin,
                        bytes=output.stat().st_size, sha256=digest(output)), output

        usage = resource.getrusage(resource.RUSAGE_CHILDREN)
        begin = time.monotonic()
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.args.jobs) as pool:
            compiled = list(pool.map(compile_source, self.manifest['sources']))
        compile_seconds = time.monotonic() - begin
        after_compile = resource.getrusage(resource.RUSAGE_CHILDREN)
        commands += [row for row, _ in compiled]
        module = directory / 'cppgm.wasm'
        command = [*runner, *flags, *[str(p.relative_to(self.inputs)) for _, p in compiled],
                   *self.manifest['link_flags'], '-lc++', '-lc++abi', '-lunwind',
                   '-o', str(module.relative_to(self.inputs))]
        begin = time.monotonic()
        self.command(command, directory / 'link.log')
        link_seconds = time.monotonic() - begin
        end = resource.getrusage(resource.RUSAGE_CHILDREN)
        cpu = lambda value: value.ru_utime + value.ru_stime
        row = dict(name=name, slot=slot, compile_seconds=compile_seconds,
                   compile_cpu_seconds=cpu(after_compile) - cpu(usage), link_seconds=link_seconds,
                   link_cpu_seconds=cpu(end) - cpu(after_compile),
                   seconds=compile_seconds + link_seconds, cpu_seconds=cpu(end) - cpu(usage),
                   sources=len(compiled), commands=commands, link_command=command,
                   module_sha256=digest(module), module_bytes=module.stat().st_size)
        outputs = {source: output for (source, (_, output)) in zip(self.manifest['sources'], compiled)}
        outputs['cppgm.wasm'] = module
        if check and self.expected is not None:
            outputs_equal(self.expected, outputs)
        row['outputs'] = {name: digest(path) for name, path in outputs.items()}
        self.rows.append(row)
        save(self.args.output / 'observations.json', self.rows)
        action = 'checked' if check and self.expected is not None else 'emitted'
        print(f'{name}: compile {compile_seconds:.3f}s, link {link_seconds:.3f}s, '
              f'CPU {row["cpu_seconds"]:.3f}s; {len(outputs)} outputs {action}', flush=True)
        return module, outputs, row


def run(args):
    args.output.mkdir(parents=True, exist_ok=False)
    args.output = args.output.resolve()
    args.tools = args.tools.resolve()
    args.compiler = args.compiler.resolve()
    if hasattr(os, 'sched_getaffinity'):
        cpus = sorted(os.sched_getaffinity(0))[:args.jobs]
        os.sched_setaffinity(0, cpus)
    else:
        cpus = []
    package = args.output / 'package'
    inception.prepare(argparse.Namespace(directory=package, sdk=args.tools / 'wasi-sdk',
                      compiler=args.compiler, shim=args.shim.resolve(), x86_headers=False, optimization=3))
    manifest = json.loads((package / 'manifest.json').read_text())
    inception.verify(package, manifest)
    campaign = Campaign(args, package, manifest)
    candidate, campaign.expected, _ = campaign.build([str(package / 'native-compiler')], 'cppgm-producer',
                                                    campaign.flags, check=False)
    reference, _, _ = campaign.build([str(args.tools / 'bin/wasm-ref-clang++')], 'clang-producer',
                                     matched_flags(manifest, clang=True), check=False)
    runners = []
    for name, module in [('cppgm', candidate), ('clang', reference)]:
        compiled = package / (name + '.cwasm')
        campaign.command([str(args.tools / 'bin/wasmtime'), 'compile', '-W', 'exceptions=y', '-W',
                          'max-wasm-stack=8388608', '-o', str(compiled), str(module)],
                         args.output / (name + '-engine-compile.log'))
        runners.append([str(args.tools / 'bin/wasmtime'), 'run', '--allow-precompiled',
                        '-W', 'exceptions=y', '-W', 'max-wasm-stack=8388608',
                        '--dir', str(campaign.inputs) + '::.', str(compiled)])
    # Warm every source and verify both producers before retaining timings.
    for slot, runner in enumerate(runners):
        campaign.build(runner, f'warm-{slot}', campaign.flags)
        shutil.rmtree(campaign.inputs / f'warm-{slot}')
    modes = {}
    for mode, blocks in [('AA', args.aa_blocks), ('AB', args.blocks)]:
        rows = []
        for block in range(blocks):
            for position, slot in enumerate([0, 1, 1, 0]):
                name = f'{mode}-{block}-{position}'
                _, _, row = campaign.build(runners[slot if mode == 'AB' else 0], name, campaign.flags, slot)
                rows.append(row)
                shutil.rmtree(campaign.inputs / name)
        modes[mode] = {metric: paired(rows, metric) for metric in
                      ['seconds', 'cpu_seconds', 'compile_seconds', 'compile_cpu_seconds',
                       'link_seconds', 'link_cpu_seconds']}
    inception.verify(package, manifest)
    metadata = dict(revision=manifest['revision'],
                    dirty=bool(subprocess.check_output(['git', 'status', '--porcelain'], cwd=ROOT)),
                    sources=len(manifest['sources']), cpu=public_benchmarks.cpu_model(), affinity=cpus,
                    jobs=args.jobs, manifest_sha256=digest(package / 'manifest.json'),
                    candidate_sha256=digest(candidate), reference_sha256=digest(reference),
                    engine_sha256=digest(args.tools / 'bin/wasmtime'),
                    clang_sha256=digest(args.tools / 'bin/wasm-ref-clang++'),
                    flags=campaign.flags, reference_producer_flags=matched_flags(manifest, True),
                    engine_version=subprocess.check_output([str(args.tools / 'bin/wasmtime'), '--version'], text=True),
                    clang_version=subprocess.check_output([str(args.tools / 'bin/wasm-ref-clang++'), '--version'], text=True),
                    system=os.uname()._asdict() if hasattr(os.uname(), '_asdict') else list(os.uname()))
    warnings = [f'{metric}: A/A ratio differs by more than 5%' for metric, value in modes['AA'].items()
                if abs(value['speedup'] - 1) > .05]
    save(args.output / 'results.json', dict(schema=1, status='complete', metadata=metadata,
                                          phases=modes, warnings=warnings))
    lines = ['# Complete WASI compiler rebuild', '',
             'Reference / cppgm; higher is better. Both WASI compilers run the same frozen full rebuild.',
             'Every object and the final module match the native cppgm producer byte for byte.',
             'The producers use identical SDK headers, EH libc++, feature macros and source inputs.',
             'Guest process/instance startup and WASI file I/O are included; engine compilation is excluded.',
             'Producer creation is a single untuned observation, retained separately in observations.json.', '',
             f'{len(manifest["sources"])} translation units; {args.jobs} jobs; CPUs {cpus}.', '',
             '| Phase / metric | cppgm-built WASI | Clang-built WASI | Ratio |',
             '|---|---:|---:|---:|']
    for metric, value in modes['AB'].items():
        lines.append(f'| {metric} | {value["candidate"]:.3f}s | {value["reference"]:.3f}s | {value["speedup"]:.3f}x |')
    lines += ['', *warnings]
    (args.output / 'summary.md').write_text('\n'.join(lines) + '\n')


def arguments():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--tools', type=Path, default=ROOT / 'obj/wasm-tools')
    parser.add_argument('--compiler', type=Path, default=ROOT / 'dev/cppgm++')
    parser.add_argument('--shim', type=Path, default=ROOT / 'scripts/experimental/node_modules/@bjorn3/browser_wasi_shim')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--blocks', type=int, default=3)
    parser.add_argument('--aa-blocks', type=int, default=1)
    parser.add_argument('--timeout', type=int, default=120, help='deadline for each compiler process')
    args = parser.parse_args()
    if min(args.jobs, args.blocks, args.aa_blocks, args.timeout) <= 0:
        parser.error('jobs, blocks and timeouts must be positive')
    return args


if __name__ == '__main__':
    run(arguments())

#!/usr/bin/env python3
"""Run calibrated perf-counter ABBA measurements on immutable compiler binaries.

Uses the repository's existing GNU-time runner and the campaign counter assessor.
Every output directory is new; observing a running experiment never restarts it.
"""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys


BATCH_RUNNER = '''#!/bin/sh
compiler=$1
repetitions=$2
shift 2
output=
previous=
for argument do
  if [ "$previous" = "-o" ]; then output="$argument"; fi
  previous="$argument"
done
if [ -z "$output" ]; then exit 2; fi
iteration=0
while [ "$iteration" -lt "$repetitions" ]; do
  iteration=$((iteration + 1))
  "$compiler" "$@" -o "$output.batch-$iteration.o" || exit $?
done
cp "$output.batch-$iteration.o" "$output"
'''


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_batch(report_path, repetitions):
    """Check every retained child output, outside the measured command."""
    observations = []
    for run in json.loads(report_path.read_text())["runs"]:
        command = run["command"]
        output = command[command.index("-o") + 1]
        for iteration in range(1, repetitions + 1):
            path = Path(f"{output}.batch-{iteration}.o")
            actual = digest(path)
            if actual != run["output_sha256"]:
                raise ValueError(f"batch child differs: {path}")
            observations.append(dict(path=str(path), sha256=actual))
    return observations


def verify_inputs(closures, sources, binaries):
    for path, expected in [*closures.items(), *sources.items(), *binaries.items()]:
        if digest(Path(path)) != expected:
            raise ValueError(f"input or binary changed: {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--frozen-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--extra-source", type=Path, action="append", default=[])
    parser.add_argument("--extra-repetitions", type=int, default=1,
                        help="compilations per observation for each short extra-source workload")
    parser.add_argument("--cpu", type=int, default=84)
    parser.add_argument("--disable-aslr", action="store_true")
    parser.add_argument("--calibration-tolerance", type=float, default=0)
    args = parser.parse_args()
    if args.extra_repetitions < 1:
        raise ValueError("extra repetitions must be positive")
    if args.cpu not in os.sched_getaffinity(0):
        raise ValueError(f"CPU {args.cpu} is not available")
    layout = ["setarch", os.uname().machine, "-R"] if args.disable_aslr else []
    if layout:
        subprocess.run([*layout, "/bin/true"], check=True)
    repo = Path(__file__).resolve().parents[1]
    frozen, output = args.frozen_root.resolve(), args.output.resolve()
    compilers = dict(a=args.baseline.resolve(), b=args.candidate.resolve())
    workloads = [
        ("overload-o0", frozen / "self_compile/stable/semantic_overload.cpp",
         [frozen / "self_compile/stable/include"], ["-std=c++11", "-O0"]),
        ("overload-o2", frozen / "self_compile/stable/semantic_overload.cpp",
         [frozen / "self_compile/stable/include"], ["-std=c++11", "-O2"]),
        ("self-calls-o0", frozen / "dev/src/sema/analyzer_calls.cpp",
         [frozen / "dev/src", frozen / "generated"], ["-std=gnu++11", "-O0"]),
    ]
    workloads.extend((p.stem + "-o0", p.resolve(), [], ["-std=c++11", "-O0"]) for p in args.extra_source)
    if len({name for name, _, _, _ in workloads}) != len(workloads):
        raise ValueError("duplicate workload names")
    closures = {}
    for closure in json.loads((frozen / "workload-closures.json").read_text()).values():
        closures.update(closure["files"])
    sources = {str(path): digest(path) for _, path, _, _ in workloads}
    identities = {str(path): digest(path) for path in compilers.values()}
    verify_inputs(closures, sources, identities)
    output.mkdir(parents=True, exist_ok=False)
    batch_runner = output / "batch-compiler"
    batch_runner.write_text(BATCH_RUNNER)
    batch_runner.chmod(0o555)
    identities[str(batch_runner)] = digest(batch_runner)
    (output / "underlying-binaries.json").write_text(json.dumps(identities, indent=2) + "\n")
    (output / "inputs.json").write_text(json.dumps(dict(closures=closures, sources=sources,
        cpu=args.cpu, disable_aslr=args.disable_aslr, calibration_tolerance=args.calibration_tolerance,
        extra_repetitions=args.extra_repetitions,
        workloads=[dict(name=n, source=str(p), includes=list(map(str, inc)), flags=flags)
                                for n, p, inc, flags in workloads]), indent=2) + "\n")
    variants = [False, True] if args.extra_repetitions > 1 else [False]
    for (label, compiler), batched in itertools.product(compilers.items(), variants):
        wrapper = output / ("compiler-" + label + ("-batch" if batched else ""))
        measured = [str(batch_runner), str(compiler), str(args.extra_repetitions)] if batched else [str(compiler)]
        wrapper.write_text('''#!/bin/sh
output=
previous=
for argument do
  if [ "$previous" = "-o" ]; then output="$argument"; fi
  previous="$argument"
done
if [ -z "$output" ]; then exit 2; fi
''' + f'exec taskset -c {args.cpu} ' + (shlex.join(layout) + ' ' if layout else '')
            + 'perf stat -x ";" -o "$output.perf" '
            '-e instructions:u,cycles:u,task-clock,context-switches,cpu-migrations,page-faults '
            + shlex.join(measured) + ' "$@"\n')
        wrapper.chmod(0o555)
    for phase in ["aa", "ab"]:
        for name, source, includes, flags in workloads:
            batched = args.extra_repetitions > 1 and source in [p.resolve() for p in args.extra_source]
            suffix = "-batch" if batched else ""
            for label in (["a"] if phase == "aa" else ["a", "b"]):
                warmup = [str(output / ("compiler-" + label + suffix)), *flags,
                          *["-I" + str(path) for path in includes], "-c", str(source),
                          "-o", str(output / "warmup.o")]
                with (output / f"{name}-{phase}-{label}-warmup.log").open("w") as log:
                    for _ in range(3):
                        subprocess.run(warmup, cwd=repo, stdout=log, stderr=log, check=True)
            command = [sys.executable, str(repo / "scripts/run_ab_compile_benchmark.py"),
                       "--compiler-a", str(output / ("compiler-a" + suffix)), "--compiler-b",
                       str(output / (("compiler-a" if phase == "aa" else "compiler-b") + suffix)),
                       "--source", str(source), "--abba-blocks", "10",
                       "--output-prefix", str(output / f"{name}-{phase}")]
            for include in includes:
                command.extend(["--include", str(include)])
            # An extra header-free workload must not inherit live dev/src includes.
            if not includes:
                command.extend(["--include", str(frozen / "self_compile/stable/include")])
            command.extend("--compiler-arg=" + flag for flag in flags)
            print(phase, name, flush=True)
            with (output / f"{name}-{phase}.log").open("w") as log:
                subprocess.run(command, cwd=repo, stdout=log, stderr=log, check=True)
            if batched:
                children = validate_batch(output / f"{name}-{phase}.json", args.extra_repetitions)
                (output / f"{name}-{phase}-batch-outputs.json").write_text(json.dumps(children, indent=2) + "\n")
        with (output / (phase + "-assessment.log")).open("w") as log:
            # Calibration gates are declared before any candidate run.
            subprocess.run([sys.executable, "-B", str(repo / "student.tests/upstream_cxx11_perf_report.py"),
                            str(output), "--phase", phase, "--expected-workloads", str(len(workloads)),
                            "--calibration-tolerance", str(args.calibration_tolerance)],
                           cwd=repo, stdout=log, stderr=log, check=True)
        verify_inputs(closures, sources, identities)
        print(phase, "all counter/memory gates passed; input hashes unchanged", flush=True)


if __name__ == "__main__":
    main()

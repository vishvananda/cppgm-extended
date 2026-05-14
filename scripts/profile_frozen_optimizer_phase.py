#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path
import re
import statistics
import subprocess
import sys
import time


PHASE_RE = re.compile(r"^phase-timing name=optimize_lowir_program(?: .*)? ms=(\d+)$")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--optimizer", default="./dev/lowiropt")
    parser.add_argument("--host-cxx",
                        default=os.environ.get("CPPGM_HOST_CXX",
                                               "/usr/local/opt/llvm/bin/clang++"))
    parser.add_argument("--source",
                        default="benchmarks/self_compile/stable/semantic_model.cpp")
    parser.add_argument("--lowir", default="",
                        help="Optional prebuilt LowIR input. Skips the source->LowIR step.")
    parser.add_argument("--include-dir", action="append", default=["dev/src"])
    parser.add_argument("--repeat", type=int, default=5)
    parser.add_argument("--level", type=int, default=2)
    parser.add_argument("--output-prefix", default="/tmp/cppgm-optimizer-phase")
    parser.add_argument("--work-dir", default="")
    return parser.parse_args()


def run_command(cmd, cwd, env):
    started = time.perf_counter()
    proc = subprocess.run(
        cmd,
        cwd=str(cwd),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return proc, (time.perf_counter() - started) * 1000.0


def extract_phase_ms(output: str):
    matches = []
    for line in output.splitlines():
        found = PHASE_RE.match(line.strip())
        if found:
            matches.append(int(found.group(1)))
    return matches


def write_markdown(path: Path, report):
    runs = report["runs"]
    lines = []
    lines.append("# Frozen Optimizer Phase Profile")
    lines.append("")
    lines.append("## Input")
    lines.append("")
    lines.append(f"- source: `{report['source']}`")
    lines.append(f"- lowir: `{report['lowir']}`")
    lines.append(f"- lowir-bytes: `{report['lowir_bytes']}`")
    lines.append(f"- optimizer: `{report['optimizer']}`")
    lines.append(f"- level: `-O{report['level']}`")
    lines.append(f"- repeat: `{report['repeat']}`")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- wall-ms min/median/max: `{report['wall_ms_min']:.2f}` / `{report['wall_ms_median']:.2f}` / `{report['wall_ms_max']:.2f}`")
    lines.append(f"- optimizer-ms min/median/max: `{report['phase_ms_min']:.2f}` / `{report['phase_ms_median']:.2f}` / `{report['phase_ms_max']:.2f}`")
    lines.append("")
    lines.append("## Runs")
    lines.append("")
    for run in runs:
        lines.append(
            f"- run `{run['index']}`: wall-ms=`{run['wall_ms']:.2f}` optimizer-ms=`{run['phase_ms']}` out=`{run['outfile']}`")
    lines.append("")
    path.write_text("\n".join(lines))


def main():
    args = parse_args()
    if args.repeat < 1:
        raise SystemExit("--repeat must be >= 1")
    if args.level < 0:
        raise SystemExit("--level must be >= 0")

    repo_root = Path(args.repo_root).resolve()
    compiler = str((repo_root / args.compiler).resolve()) if args.compiler.startswith(".") else args.compiler
    optimizer = str((repo_root / args.optimizer).resolve()) if args.optimizer.startswith(".") else args.optimizer
    source = (repo_root / args.source).resolve()
    if args.lowir:
        lowir_path = Path(args.lowir)
        if not lowir_path.is_absolute():
            lowir_path = (repo_root / lowir_path).resolve()
        if not lowir_path.is_file():
            raise SystemExit("missing lowir: " + args.lowir)
    else:
        if not source.is_file():
            raise SystemExit("missing source: " + args.source)

    output_prefix = Path(args.output_prefix)
    output_prefix.parent.mkdir(parents=True, exist_ok=True)
    work_dir = Path(args.work_dir) if args.work_dir else Path(str(output_prefix) + ".work")
    if not work_dir.is_absolute():
        work_dir = (repo_root / work_dir).resolve()
    work_dir.mkdir(parents=True, exist_ok=True)

    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = args.host_cxx
    env["CPPGM_PHASE_TIMING"] = "1"

    compile_wall_ms = 0.0
    if not args.lowir:
        lowir_path = work_dir / (source.stem + ".o0.lowir")
        include_flags = []
        for include_dir in args.include_dir:
            include_flags.extend(["-I", include_dir])
        compile_cmd = [compiler, "--emit-lowir", "-O0", *include_flags, "-o", str(lowir_path),
                       str(source)]
        compile_proc, compile_wall_ms = run_command(compile_cmd, repo_root, env)
        if compile_proc.returncode != 0:
            sys.stderr.write(compile_proc.stdout)
            raise SystemExit(compile_proc.returncode)

    runs = []
    for index in range(1, args.repeat + 1):
        out_path = work_dir / f"{source.stem}.opt.run{index}.lowir"
        cmd = [optimizer, f"-O{args.level}", "-o", str(out_path), str(lowir_path)]
        proc, wall_ms = run_command(cmd, repo_root, env)
        if proc.returncode != 0:
            sys.stderr.write(proc.stdout)
            raise SystemExit(proc.returncode)
        phase_matches = extract_phase_ms(proc.stdout)
        if len(phase_matches) != 1:
            sys.stderr.write(proc.stdout)
            raise SystemExit("expected exactly one optimize_lowir_program phase-timing line")
        runs.append({
            "index": index,
            "wall_ms": wall_ms,
            "phase_ms": phase_matches[0],
            "outfile": str(out_path),
        })
        print(f"[{index}/{args.repeat}] wall-ms={wall_ms:.2f} optimizer-ms={phase_matches[0]}",
              flush=True)

    wall_values = [run["wall_ms"] for run in runs]
    phase_values = [run["phase_ms"] for run in runs]
    report = {
        "source": str(source.relative_to(repo_root)),
        "lowir": str(lowir_path),
        "lowir_bytes": lowir_path.stat().st_size,
        "compiler": compiler,
        "optimizer": optimizer,
        "host_cxx": args.host_cxx,
        "repeat": args.repeat,
        "level": args.level,
        "compile_wall_ms": compile_wall_ms,
        "wall_ms_min": min(wall_values),
        "wall_ms_median": statistics.median(wall_values),
        "wall_ms_max": max(wall_values),
        "phase_ms_min": min(phase_values),
        "phase_ms_median": statistics.median(phase_values),
        "phase_ms_max": max(phase_values),
        "runs": runs,
    }

    json_path = Path(str(output_prefix) + ".json")
    md_path = Path(str(output_prefix) + ".md")
    json_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    write_markdown(md_path, report)

    print(f"Wrote {json_path}", flush=True)
    print(f"Wrote {md_path}", flush=True)


if __name__ == "__main__":
    main()

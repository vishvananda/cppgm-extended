#!/usr/bin/env python3

import argparse
import json
import os
import shutil
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Dict, List, Optional


BENCHMARKS = {
    "semantic_expression": Path("dev/src/semantic_expression.cpp"),
    "semantic_lookup": Path("dev/src/semantic_lookup.cpp"),
}


def repo_root() -> Path:
    return Path(__file__).resolve().parent.parent


def git_short_head(root: Path) -> str:
    return subprocess.check_output(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=root,
        text=True,
    ).strip()


def ensure_dir(path: Path) -> None:
    path.mkdir(parents=True, exist_ok=True)


def resolve_clang_cxx() -> str:
    env = os.environ.get("CPPGM_HOST_CXX")
    candidates = []
    if env:
        candidates.append(env)
    candidates.extend(
        [
            "/opt/homebrew/opt/llvm/bin/clang++",
            "/usr/local/opt/llvm/bin/clang++",
            "clang++-22",
            "clang++",
            "c++",
        ]
    )
    for candidate in candidates:
        resolved = shutil.which(candidate) if "/" not in candidate else candidate
        if resolved and Path(resolved).exists():
            return resolved
    raise SystemExit("Unable to resolve host clang++ for memory benchmark")


def freeze_sources(args: argparse.Namespace) -> int:
    root = repo_root()
    commit = args.commit or git_short_head(root)
    out_root = Path(args.root).expanduser() / commit
    ensure_dir(out_root)

    manifest = {
        "commit": commit,
        "repo_root": str(root),
        "benchmarks": {},
    }
    for name, relative in BENCHMARKS.items():
        src = root / relative
        dst = out_root / relative.name
        shutil.copy2(src, dst)
        manifest["benchmarks"][name] = {
            "source": str(src),
            "frozen": str(dst),
        }

    (out_root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(out_root)
    return 0


def build_cppgm(root: Path) -> None:
    subprocess.run(
        ["make", "-j1", "-C", "dev", "cppgm++", "CPPGM_TEST_RUNNER=1"],
        cwd=root,
        check=True,
    )


def read_ps_snapshot(pid: int) -> Optional[Dict[str, object]]:
    try:
        out = subprocess.check_output(
            [
                "ps",
                "-o",
                "pid=,rss=,vsz=,etime=,state=,command=",
                "-p",
                str(pid),
            ],
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()
    except subprocess.CalledProcessError:
        return None
    if not out:
        return None
    parts = out.split(None, 5)
    if len(parts) < 6:
        return None
    return {
        "pid": int(parts[0]),
        "rss_kb": int(parts[1]),
        "vsz_kb": int(parts[2]),
        "etime": parts[3],
        "state": parts[4],
        "command": parts[5],
    }


def monitor_process(pid: int,
                    poll_interval_sec: float,
                    stop_event: threading.Event,
                    timeline: List[Dict[str, object]]) -> None:
    start = time.perf_counter()
    while not stop_event.is_set():
        snapshot = read_ps_snapshot(pid)
        if snapshot is not None:
            snapshot["elapsed_seconds"] = time.perf_counter() - start
            timeline.append(snapshot)
        time.sleep(poll_interval_sec)


def run_optional_diagnostics(pid: int,
                             result_dir: Path,
                             sample_seconds: int) -> None:
    sample_cmd = shutil.which("sample")
    if sample_cmd:
        subprocess.run(
            [
                sample_cmd,
                str(pid),
                str(sample_seconds),
                "1",
                "-mayDie",
                "-file",
                str(result_dir / "sample.txt"),
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
    vmmap_cmd = shutil.which("vmmap")
    if vmmap_cmd:
        with (result_dir / "vmmap.txt").open("w", encoding="utf-8") as out:
            subprocess.run(
                [vmmap_cmd, str(pid)],
                stdout=out,
                stderr=subprocess.DEVNULL,
                check=False,
            )


def benchmark_command(root: Path,
                      tool: str,
                      source: Path,
                      output: Path) -> list[str]:
    include_flag = f"-I{root / 'dev/src'}"
    if tool == "clang":
        return [
            resolve_clang_cxx(),
            "-std=gnu++11",
            "-Wall",
            "-O3",
            include_flag,
            "-c",
            "-o",
            str(output),
            str(source),
        ]
    if tool == "cppgm":
        return [
            str(root / "dev/cppgm++"),
            include_flag,
            "-c",
            "-o",
            str(output),
            str(source),
        ]
    raise ValueError(f"unknown tool: {tool}")


def run_benchmark(args: argparse.Namespace) -> int:
    root = repo_root()
    commit = args.commit or git_short_head(root)
    frozen_root = Path(args.root).expanduser() / commit
    result_dir = Path(args.results_root).expanduser() / commit / f"{args.bench}.{args.tool}"
    ensure_dir(result_dir)

    source = frozen_root / BENCHMARKS[args.bench].name
    if not source.exists():
        raise SystemExit(
            f"Frozen benchmark source not found: {source}\n"
            f"Run `python3 scripts/selfhost_memory_bench.py freeze` first."
        )

    if args.build_cppgm:
        build_cppgm(root)

    output = result_dir / f"{args.bench}.{args.tool}.o"
    stdout_path = result_dir / "compile.stdout"
    stderr_path = result_dir / "compile.stderr"
    command_path = result_dir / "command.txt"
    summary_path = result_dir / "summary.json"
    timeline_path = result_dir / "ps_timeline.json"

    command = benchmark_command(root, args.tool, source, output)
    command_path.write_text(" ".join(command) + "\n", encoding="utf-8")

    with stdout_path.open("w", encoding="utf-8") as stdout_file, stderr_path.open(
        "w", encoding="utf-8"
    ) as stderr_file:
        start = time.perf_counter()
        proc = subprocess.Popen(
            command,
            cwd=root,
            stdout=stdout_file,
            stderr=stderr_file,
            text=True,
        )
        timeline: List[Dict[str, object]] = []
        stop_event = threading.Event()
        monitor = threading.Thread(
            target=monitor_process,
            args=(proc.pid, args.poll_interval_sec, stop_event, timeline),
            daemon=True,
        )
        monitor.start()

        if args.collect_diagnostics:
            run_optional_diagnostics(proc.pid, result_dir, args.sample_seconds)

        returncode = proc.wait()
        stop_event.set()
        monitor.join(timeout=max(1.0, args.poll_interval_sec * 2.0))
        wall_seconds = time.perf_counter() - start

    timeline_path.write_text(json.dumps(timeline, indent=2) + "\n", encoding="utf-8")
    max_rss_kb = max((entry["rss_kb"] for entry in timeline), default=0)

    summary = {
        "tool": args.tool,
        "bench": args.bench,
        "commit": commit,
        "source": str(source),
        "output": str(output),
        "returncode": returncode,
        "wall_seconds": wall_seconds,
        "max_observed_rss_kb": max_rss_kb,
        "timeline_samples": len(timeline),
        "collect_diagnostics": args.collect_diagnostics,
    }
    summary_path.write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")

    print(json.dumps(summary, indent=2))
    return returncode


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Freeze and measure isolated self-host memory benchmark translation units."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    freeze = subparsers.add_parser("freeze", help="Freeze the benchmark translation units by commit.")
    freeze.add_argument("--root", default="/tmp/cppgm-memory-bench")
    freeze.add_argument("--commit")
    freeze.set_defaults(func=freeze_sources)

    run = subparsers.add_parser("run", help="Compile one frozen benchmark file and record measurements.")
    run.add_argument("--tool", choices=("clang", "cppgm"), required=True)
    run.add_argument("--bench", choices=tuple(BENCHMARKS.keys()), required=True)
    run.add_argument("--root", default="/tmp/cppgm-memory-bench")
    run.add_argument("--results-root", default="/tmp/cppgm-memory-bench-results")
    run.add_argument("--commit")
    run.add_argument("--build-cppgm", action="store_true")
    run.add_argument("--collect-diagnostics", action="store_true")
    run.add_argument("--sample-seconds", type=int, default=5)
    run.add_argument("--poll-interval-sec", type=float, default=0.5)
    run.set_defaults(func=run_benchmark)

    return parser


def main(argv: List[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

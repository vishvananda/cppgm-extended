#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import time
from typing import Dict, List


def split_env(items: List[str]) -> Dict[str, str]:
    out: Dict[str, str] = {}
    for item in items:
        if "=" not in item:
            raise SystemExit(f"--env expects NAME=VALUE, got {item!r}")
        key, value = item.split("=", 1)
        if not key:
            raise SystemExit(f"--env expects NAME=VALUE, got {item!r}")
        out[key] = value
    return out


def median(values: List[float]) -> float:
    return float(statistics.median(values))


def average(values: List[float]) -> float:
    return float(sum(values) / len(values))


def run_one(repo_root: Path,
            compiler: str,
            source: str,
            label: str,
            index: int,
            env: Dict[str, str],
            timeout_sec: int,
            output_dir: Path) -> Dict[str, object]:
    output_dir.mkdir(parents=True, exist_ok=True)
    out = output_dir / f"{label}-{index}.o"
    cmd = [compiler, "-I", "dev/src", "-c", "-o", str(out), source]
    started = time.perf_counter()
    proc = subprocess.run(
        cmd,
        cwd=str(repo_root),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        timeout=None if timeout_sec <= 0 else timeout_sec,
    )
    wall = time.perf_counter() - started
    return {
        "label": label,
        "index": index,
        "cmd": cmd,
        "status": "ok" if proc.returncode == 0 else "error",
        "returncode": proc.returncode,
        "wall_seconds": wall,
        "stdout": proc.stdout,
    }


def summarize(label: str, runs: List[Dict[str, object]]) -> Dict[str, object]:
    ok = [r for r in runs if r["status"] == "ok"]
    walls = [float(r["wall_seconds"]) for r in ok]
    return {
        "label": label,
        "runs": len(runs),
        "ok": len(ok),
        "median_wall_seconds": median(walls) if walls else None,
        "average_wall_seconds": average(walls) if walls else None,
        "min_wall_seconds": min(walls) if walls else None,
        "max_wall_seconds": max(walls) if walls else None,
    }


def write_markdown(path: Path, report: Dict[str, object]) -> None:
    summary = {item["label"]: item for item in report["summary"]}
    labels = report["labels"]
    a = summary[labels[0]]
    b = summary[labels[1]]
    avg_delta = None
    med_delta = None
    if a["average_wall_seconds"] and b["average_wall_seconds"]:
        avg_delta = ((b["average_wall_seconds"] - a["average_wall_seconds"]) /
                     a["average_wall_seconds"]) * 100.0
    if a["median_wall_seconds"] and b["median_wall_seconds"]:
        med_delta = ((b["median_wall_seconds"] - a["median_wall_seconds"]) /
                     a["median_wall_seconds"]) * 100.0

    lines = [
        "# A/B Compile Benchmark",
        "",
        f"- source: `{report['source']}`",
        f"- baseline: `{report['compiler_a']}`",
        f"- candidate: `{report['compiler_b']}`",
        f"- repeat-pairs: `{report['repeat_pairs']}`",
        "",
        "| Label | OK | Median | Average | Min | Max |",
        "| --- | ---: | ---: | ---: | ---: | ---: |",
    ]
    for item in report["summary"]:
        def fmt(v):
            return "n/a" if v is None else f"{float(v):.3f}s"
        lines.append(
            f"| `{item['label']}` | {item['ok']}/{item['runs']} | "
            f"{fmt(item['median_wall_seconds'])} | "
            f"{fmt(item['average_wall_seconds'])} | "
            f"{fmt(item['min_wall_seconds'])} | "
            f"{fmt(item['max_wall_seconds'])} |")
    lines.extend([
        "",
        "## Delta",
        "",
        f"- median candidate-vs-baseline: `{med_delta:+.2f}%`" if med_delta is not None else
        "- median candidate-vs-baseline: `n/a`",
        f"- average candidate-vs-baseline: `{avg_delta:+.2f}%`" if avg_delta is not None else
        "- average candidate-vs-baseline: `n/a`",
        "",
        "## Sequence",
        "",
    ])
    for run in report["runs"]:
        lines.append(
            f"- {run['index']:02d} `{run['label']}` `{run['status']}` "
            f"{float(run['wall_seconds']):.3f}s")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run alternating A/B compile benchmarks against two binaries."
    )
    parser.add_argument("--repo-root", default=str(Path(__file__).resolve().parent.parent))
    parser.add_argument("--source", default="pa34/tests/compile/540-reference-wrapper-smoke.t")
    parser.add_argument("--compiler-a", required=True)
    parser.add_argument("--compiler-b", required=True)
    parser.add_argument("--label-a", default="baseline")
    parser.add_argument("--label-b", default="candidate")
    parser.add_argument("--repeat-pairs", type=int, default=5)
    parser.add_argument("--timeout-sec", type=int, default=180)
    parser.add_argument("--env", action="append", default=[])
    parser.add_argument("--output-prefix", default="/tmp/cppgm-ab-compile")
    args = parser.parse_args()

    if args.repeat_pairs < 1:
        raise SystemExit("--repeat-pairs must be >= 1")

    repo_root = Path(args.repo_root).resolve()
    env = os.environ.copy()
    env.update(split_env(args.env))
    output_dir = Path(args.output_prefix + "-objects")
    runs: List[Dict[str, object]] = []
    sequence = [
        (args.label_a, args.compiler_a),
        (args.label_b, args.compiler_b),
        (args.label_b, args.compiler_b),
        (args.label_a, args.compiler_a),
    ]
    for pair in range(args.repeat_pairs):
        for label, compiler in sequence:
            index = len(runs) + 1
            print(f"run {index}: {label}", flush=True)
            run = run_one(repo_root,
                          compiler,
                          args.source,
                          label,
                          index,
                          env,
                          args.timeout_sec,
                          output_dir)
            print(f"  {run['status']} {float(run['wall_seconds']):.3f}s", flush=True)
            runs.append(run)

    labels = [args.label_a, args.label_b]
    report = {
        "source": args.source,
        "compiler_a": args.compiler_a,
        "compiler_b": args.compiler_b,
        "labels": labels,
        "repeat_pairs": args.repeat_pairs,
        "runs": runs,
        "summary": [
            summarize(args.label_a, [r for r in runs if r["label"] == args.label_a]),
            summarize(args.label_b, [r for r in runs if r["label"] == args.label_b]),
        ],
    }

    json_path = Path(args.output_prefix + ".json")
    md_path = Path(args.output_prefix + ".md")
    json_path.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")
    write_markdown(md_path, report)
    print(f"Wrote {json_path}")
    print(f"Wrote {md_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

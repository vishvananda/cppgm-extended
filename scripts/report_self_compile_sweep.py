#!/usr/bin/env python3

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from bootstrap_selfhost_layout import compile_command
from bootstrap_selfhost_layout import object_path
from bootstrap_selfhost_layout import source_list


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--host-cxx",
                        default=os.environ.get("CPPGM_HOST_CXX",
                                               "/usr/local/opt/llvm/bin/clang++"))
    parser.add_argument("--frontend", default="dev/cppgm++.cpp")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--jobs", type=int, default=max(os.cpu_count() or 1, 1))
    parser.add_argument("--timeout-sec", type=int, default=30)
    parser.add_argument("--output-prefix", default="/tmp/bootstrap-self-compile-sweep")
    parser.add_argument("--build-dir", default="",
                        help="Optional object-root directory. Defaults to <output-prefix>.build")
    parser.add_argument("--object-layout", default="scratch",
                        choices=["scratch", "pa39-selfhost"],
                        help="How to place compiled objects on disk")
    parser.add_argument("--test-runner", type=int,
                        default=int(os.environ.get("CPPGM_TEST_RUNNER", "1") or "1"),
                        help="Whether PA39 layout uses runner-flavored entry/shared object names")
    parser.add_argument("--stdlib-flags",
                        default=os.environ.get("CPPGM_STDLIB_FLAGS", ""),
                        help="Optional stdlib flags mirrored into the compile commands")
    parser.add_argument("--source", action="append", default=[],
                        help="Optional repo-relative source path(s) to sweep instead of the full set")
    parser.add_argument("--slow-ok-limit", type=int, default=15)
    return parser.parse_args()


def requested_sources(repo_root: Path, args):
    if not args.source:
        return source_list(repo_root, args.frontend, args.object_layout, bool(args.test_runner))
    out = []
    seen = set()
    for item in args.source:
        path = (repo_root / item).resolve()
        if not path.is_file():
            raise SystemExit("missing source: " + item)
        if path in seen:
            continue
        seen.add(path)
        out.append(path)
    return out


def normalize_failure_output(text: str):
    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue
        line = re.sub(r":[0-9]+:[0-9]+", ":<loc>", line)
        line = re.sub(r"/tmp/[^ :]+", "<tmp>", line)
        line = re.sub(r"/private/tmp/[^ :]+", "<tmp>", line)
        line = re.sub(r"\bdev/src/[^ :]+", "<src>", line)
        line = re.sub(r"\s+", " ", line)
        return line[:240]
    return "<no output>"


def compile_source(repo_root: Path,
                   compiler: str,
                   src: Path,
                   obj: Path,
                   layout: str,
                   test_runner: bool,
                   stdlib_flags: str,
                   env,
                   timeout_sec: int):
    cmd = compile_command(repo_root, compiler, src, obj, layout, test_runner, stdlib_flags)
    obj.parent.mkdir(parents=True, exist_ok=True)
    started = time.time()
    try:
        timeout_value = timeout_sec if timeout_sec > 0 else None
        proc = subprocess.run(
            cmd,
            cwd=str(repo_root),
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=timeout_value,
        )
        stdout = proc.stdout
        returncode = proc.returncode
        status = "ok" if proc.returncode == 0 else "error"
    except subprocess.TimeoutExpired as exc:
        stdout = exc.stdout or ""
        returncode = 124
        status = "timeout"
    return {
        "cmd": cmd,
        "returncode": returncode,
        "status": status,
        "stdout": stdout,
        "duration_sec": time.time() - started,
    }


def make_report(args, repo_root: Path, build_dir: Path, results):
    results.sort(key=lambda item: item["source"])
    timeout_results = [r for r in results if r["status"] == "timeout"]
    error_results = [r for r in results if r["status"] == "error"]
    ok_results = [r for r in results if r["status"] == "ok"]

    error_clusters = {}
    for stage in error_results:
        signature = normalize_failure_output(stage.get("stdout", ""))
        cluster = error_clusters.setdefault(signature, {
            "count": 0,
            "examples": [],
        })
        cluster["count"] += 1
        if len(cluster["examples"]) < 5:
            cluster["examples"].append(stage["source"])

    report = {
        "compiler": args.compiler,
        "frontend": args.frontend,
        "host_cxx": args.host_cxx,
        "jobs": args.jobs,
        "timeout_sec": args.timeout_sec,
        "output_prefix": str(Path(args.output_prefix)),
        "build_dir": str(build_dir),
        "object_layout": args.object_layout,
        "total": len(results),
        "ok": len(ok_results),
        "timeout": len(timeout_results),
        "error": len(error_results),
        "results": results,
        "error_clusters": [
            {
                "signature": signature,
                "count": cluster["count"],
                "examples": cluster["examples"],
            }
            for signature, cluster in sorted(
                error_clusters.items(),
                key=lambda item: (-item[1]["count"], item[0]),
            )
        ],
        "slow_ok": sorted(ok_results,
                           key=lambda item: item["duration_sec"],
                           reverse=True)[:max(args.slow_ok_limit, 0)],
    }
    return report


def write_markdown(path: Path, report):
    lines = []
    lines.append("# Self-Compile Sweep")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- total: `{report['total']}`")
    lines.append(f"- ok: `{report['ok']}`")
    lines.append(f"- timeout: `{report['timeout']}`")
    lines.append(f"- error: `{report['error']}`")
    timeout_label = "none" if report["timeout_sec"] == 0 else str(report["timeout_sec"])
    lines.append(f"- timeout-sec: `{timeout_label}`")
    lines.append(f"- jobs: `{report['jobs']}`")
    lines.append("")

    lines.append("## Timeouts")
    lines.append("")
    if report["timeout"] == 0:
        lines.append("- none")
    else:
        for stage in report["results"]:
            if stage["status"] != "timeout":
                continue
            lines.append(
                f"- `{stage['source']}` ({stage['duration_sec']:.1f}s): "
                f"`{normalize_failure_output(stage.get('stdout', ''))}`")
    lines.append("")

    lines.append("## Error Clusters")
    lines.append("")
    if not report["error_clusters"]:
        lines.append("- none")
    else:
        for cluster in report["error_clusters"]:
            examples = ", ".join(f"`{item}`" for item in cluster["examples"])
            lines.append(
                f"- `{cluster['signature']}`: {cluster['count']} file(s)"
                + (f" ({examples})" if examples else ""))
    lines.append("")

    lines.append("## Error Files")
    lines.append("")
    if report["error"] == 0:
        lines.append("- none")
    else:
        for stage in report["results"]:
            if stage["status"] != "error":
                continue
            lines.append(
                f"- `{stage['source']}` ({stage['duration_sec']:.1f}s): "
                f"`{normalize_failure_output(stage.get('stdout', ''))}`")
    lines.append("")

    lines.append("## Slow Successful Files")
    lines.append("")
    if not report["slow_ok"]:
        lines.append("- none")
    else:
        for stage in report["slow_ok"]:
            lines.append(f"- `{stage['source']}` ({stage['duration_sec']:.1f}s)")
    lines.append("")

    path.write_text("\n".join(lines))


def main():
    args = parse_args()
    if args.jobs < 1:
        raise SystemExit("--jobs must be >= 1")
    if args.timeout_sec < 0:
        raise SystemExit("--timeout-sec must be >= 0")

    repo_root = Path(args.repo_root).resolve()
    sources = requested_sources(repo_root, args)
    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = args.host_cxx

    output_prefix = Path(args.output_prefix)
    output_prefix.parent.mkdir(parents=True, exist_ok=True)
    if args.build_dir:
        build_dir = Path(args.build_dir)
        if not build_dir.is_absolute():
            build_dir = repo_root / build_dir
    else:
        build_dir = Path(str(output_prefix) + ".build")
    build_dir.mkdir(parents=True, exist_ok=True)

    results = []
    print(f"Starting self-compile sweep for {len(sources)} files", flush=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = {}
        for index, src in enumerate(sources):
            obj = object_path(repo_root, build_dir, index, src, args.object_layout, bool(args.test_runner))
            future = executor.submit(
                compile_source,
                repo_root,
                args.compiler,
                src,
                obj,
                args.object_layout,
                bool(args.test_runner),
                args.stdlib_flags,
                env,
                args.timeout_sec,
            )
            futures[future] = (index, src, obj)

        done = 0
        for future in as_completed(futures):
            index, src, obj = futures[future]
            result = future.result()
            done += 1
            result["source"] = str(src.relative_to(repo_root))
            result["object"] = str(obj)
            result["summary"] = normalize_failure_output(result.get("stdout", ""))
            results.append(result)
            print(
                f"[{done}/{len(sources)}] {result['status']:7s} "
                f"{result['source']} ({result['duration_sec']:.1f}s) "
                f"{result['summary']}",
                flush=True,
            )

    report = make_report(args, repo_root, build_dir, results)
    json_path = Path(str(output_prefix) + ".json")
    md_path = Path(str(output_prefix) + ".md")
    json_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    write_markdown(md_path, report)

    print(f"Wrote {json_path}")
    print(f"Wrote {md_path}")
    print(
        f"SUMMARY ok={report['ok']} timeout={report['timeout']} error={report['error']}",
        flush=True,
    )


if __name__ == "__main__":
    main()

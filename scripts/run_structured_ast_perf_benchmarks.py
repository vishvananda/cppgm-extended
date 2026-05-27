#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import threading
import time
from typing import Dict, List, Optional


DEFAULT_BENCHMARKS = [
    {
        "name": "pa10-template-function-body-ast",
        "source": "pa10/tests/general/205-template-function-body.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "template-body"],
    },
    {
        "name": "pa10-template-class-body-ast",
        "source": "pa10/tests/general/206-template-class-body.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "class-template"],
    },
    {
        "name": "pa10-forward-unknown-nested-template-ast",
        "source": "pa10/tests/general/243-forward-unknown-nested-template-in-ctor-body.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "nested-template"],
    },
    {
        "name": "pa10-member-template-less-greater-ast",
        "source": "pa10/tests/general/244-member-template-if-less-template-call.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "angle-disambiguation"],
    },
    {
        "name": "pa10-template-function-pointer-ast",
        "source": "pa10/tests/general/246-template-id-function-pointer-argument.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "template-id"],
    },
    {
        "name": "pa10-dependent-template-keyword-ast",
        "source": "pa10/tests/general/270-dependent-template-keyword-nested-angle.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "nested-angle"],
    },
    {
        "name": "pa10-nested-qualified-template-id-ast",
        "source": "pa10/tests/general/271-nested-qualified-template-id-template-args.t",
        "mode": "emit-ast",
        "tags": ["pa10", "ast", "qualified-template-id"],
    },
    {
        "name": "pa18-template-shift-stress-lowir",
        "source": "pa18/tests/general/192-template-operator-shift-stress-chain.t",
        "mode": "emit-lowir",
        "tags": ["pa18", "lowir", "operator-template"],
    },
    {
        "name": "pa18-pack-forward-lowir",
        "source": "pa18/tests/general/203-function-template-pack-forward-call.t",
        "mode": "emit-lowir",
        "tags": ["pa18", "lowir", "pack"],
    },
    {
        "name": "pa18-repeated-template-call-lowir",
        "source": "pa18/tests/general/222-repeated-implicit-function-template-call.t",
        "mode": "emit-lowir",
        "tags": ["pa18", "lowir", "template-cache"],
    },
    {
        "name": "pa21-inline-class-template-member-lowir",
        "source": "pa21/tests/general/420-inline-class-template-member-required-output.t",
        "mode": "emit-lowir",
        "tags": ["pa21", "lowir", "class-template"],
    },
    {
        "name": "pa21-local-variable-template-lowir",
        "source": "pa21/tests/general/438-local-variable-template-keeps-concrete-class-instantiation.t",
        "mode": "emit-lowir",
        "tags": ["pa21", "lowir", "variable-template"],
    },
    {
        "name": "pa21-partial-specialization-alias-lowir",
        "source": "pa21/tests/general/437-partial-specialization-alias-pattern.t",
        "mode": "emit-lowir",
        "tags": ["pa21", "lowir", "alias-template"],
    },
]


FOCUSED_PA34_PERF_BENCHMARKS = [
    {
        "name": "pa34-reference-wrapper-smoke",
        "source": "pa34/tests/compile/540-reference-wrapper-smoke.t",
        "mode": "compile",
        "tags": ["pa34", "long", "reference-wrapper", "functional"],
    },
    {
        "name": "pa34-long-unordered-map-find",
        "source": "pa34/tests/compile/655-const-unordered-map-find.t",
        "mode": "compile",
        "tags": ["pa34", "long", "unordered-map", "string"],
        "allow_failure": True,
    },
    {
        "name": "pa34-long-istream-static-member-mask",
        "source": "pa34/tests/compile/658-istream-static-member-mask-access.t",
        "mode": "compile",
        "tags": ["pa34", "long", "iostream", "class-template"],
        "allow_failure": True,
    },
    {
        "name": "pa34-long-ostringstream-unsigned-int",
        "source": "pa34/tests/compile/662-hosted-ostringstream-unsigned-int.t",
        "mode": "compile",
        "tags": ["pa34", "long", "iostream", "templates"],
        "allow_failure": True,
    },
    {
        "name": "pa34-long-vector-bool-storage",
        "source": "pa34/tests/compile/679-hosted-vector-bool-storage-allocator-static-cast.t",
        "mode": "compile",
        "tags": ["pa34", "long", "vector", "allocator"],
        "allow_failure": True,
    },
    {
        "name": "pa34-long-recursive-std-function-string",
        "source": "pa34/tests/compile/680-hosted-recursive-std-function-string-substr.t",
        "mode": "compile",
        "tags": ["pa34", "long", "std-function", "string"],
        "allow_failure": True,
    },
]


OPTIONAL_SELF_COMPILE_BENCHMARKS = [
    {
        "name": "self-constant-value",
        "source": "benchmarks/self_compile/stable/constant_value.cpp",
        "mode": "compile",
        "tags": ["self-compile", "control"],
    },
    {
        "name": "self-cppast-dump",
        "source": "benchmarks/self_compile/stable/cppast_dump.cpp",
        "mode": "compile",
        "tags": ["self-compile", "ast"],
    },
    {
        "name": "self-parser-trace",
        "source": "benchmarks/self_compile/stable/parser_trace.cpp",
        "mode": "compile",
        "tags": ["self-compile", "parser"],
    },
    {
        "name": "self-semantic-overload",
        "source": "benchmarks/self_compile/stable/semantic_overload.cpp",
        "mode": "compile",
        "tags": ["self-compile", "semantic", "overload", "stl"],
        "allow_failure": True,
    },
]


CHECKPOINT_SELF_COMPILE_BENCHMARKS = [
    {
        "name": "self-callsemantic-frozen",
        "source": "benchmarks/self_compile/stable/callsemantic_frozen.cpp",
        "mode": "compile",
        "tags": ["self-compile", "semantic", "checkpoint"],
    },
]


ALL_BENCHMARKS = (
    DEFAULT_BENCHMARKS +
    FOCUSED_PA34_PERF_BENCHMARKS +
    OPTIONAL_SELF_COMPILE_BENCHMARKS +
    CHECKPOINT_SELF_COMPILE_BENCHMARKS
)


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parent.parent


def git_head(repo_root: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=str(repo_root),
            text=True,
        ).strip()
    except subprocess.CalledProcessError:
        return ""


def git_branch(repo_root: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "branch", "--show-current"],
            cwd=str(repo_root),
            text=True,
        ).strip()
    except subprocess.CalledProcessError:
        return ""


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


def median(values: List[float]) -> Optional[float]:
    if not values:
        return None
    return float(statistics.median(values))


def benchmark_map() -> Dict[str, Dict[str, object]]:
    return {item["name"]: item for item in ALL_BENCHMARKS}


def selected_benchmarks(names: List[str]) -> List[Dict[str, object]]:
    by_name = benchmark_map()
    if not names:
        return list(DEFAULT_BENCHMARKS)
    out = []
    for name in names:
        if name not in by_name:
            raise SystemExit(f"unknown benchmark: {name}")
        out.append(by_name[name])
    return out


def split_env_assignments(assignments: List[str]) -> Dict[str, str]:
    out = {}
    for item in assignments:
        if "=" not in item:
            raise SystemExit(f"--env expects NAME=VALUE, got: {item}")
        key, value = item.split("=", 1)
        if not key:
            raise SystemExit(f"--env expects NAME=VALUE, got: {item}")
        out[key] = value
    return out


def parse_counter_value(value: str) -> object:
    if value and (value.isdigit() or (value[0] == "-" and value[1:].isdigit())):
        return int(value)
    return value


def parse_key_value_tokens(text: str) -> Dict[str, object]:
    out: Dict[str, object] = {}
    for token in text.split():
        if "=" not in token:
            continue
        key, value = token.split("=", 1)
        if key:
            out[key] = parse_counter_value(value)
    return out


def parse_hotspot_query_row(line: str) -> Dict[str, object]:
    stripped = line.strip()
    prefix, separator, text = stripped.partition(" text=")
    out = parse_key_value_tokens(prefix)
    if separator:
        out["text"] = text
    return out


def parse_counter_output(stdout: str) -> Dict[str, object]:
    semantic_metrics: Dict[str, object] = {}
    semantic_cache_counters: Dict[str, object] = {}
    semantic_cache_sizes: Dict[str, object] = {}
    semantic_hotspot_summary: Dict[str, object] = {}
    semantic_hotspot_queries: List[Dict[str, object]] = []
    semantic_reachability: Dict[str, object] = {}
    semantic_reachability_queues: List[Dict[str, object]] = []
    semantic_memory: Dict[str, Dict[str, object]] = {}
    semantic_memory_details: Dict[str, Dict[str, object]] = {}
    semantic_memory_total: Dict[str, object] = {}

    in_semantic_queries = False
    for raw_line in stdout.splitlines():
        line = raw_line.rstrip()
        if line.startswith("semantic-metrics"):
            semantic_metrics.update(
                parse_key_value_tokens(line[len("semantic-metrics"):].strip()))
            in_semantic_queries = False
            continue
        if line.startswith("semantic-cache"):
            values = parse_key_value_tokens(line[len("semantic-cache"):].strip())
            name = values.get("name")
            if name is not None and ("hits" in values or "misses" in values):
                cache_values = dict(values)
                del cache_values["name"]
                semantic_cache_counters[str(name)] = cache_values
            else:
                semantic_cache_sizes.update(values)
            in_semantic_queries = False
            continue
        if line.startswith("SEMANTIC_HOTSPOT summary"):
            semantic_hotspot_summary.update(
                parse_key_value_tokens(line[len("SEMANTIC_HOTSPOT summary"):].strip()))
            in_semantic_queries = False
            continue
        if line == "SEMANTIC_HOTSPOT semantic_queries":
            in_semantic_queries = True
            continue
        if line.startswith("SEMANTIC_HOTSPOT "):
            in_semantic_queries = False
            continue
        if line.startswith("semantic-reachability-queue"):
            semantic_reachability_queues.append(
                parse_key_value_tokens(line[len("semantic-reachability-queue"):].strip()))
            in_semantic_queries = False
            continue
        if line.startswith("semantic-reachability"):
            semantic_reachability.update(
                parse_key_value_tokens(line[len("semantic-reachability"):].strip()))
            in_semantic_queries = False
            continue
        if line.startswith("semantic-memory-total"):
            semantic_memory_total.update(
                parse_key_value_tokens(line[len("semantic-memory-total"):].strip()))
            in_semantic_queries = False
            continue
        if line.startswith("semantic-memory-detail"):
            values = parse_key_value_tokens(line[len("semantic-memory-detail"):].strip())
            name = values.get("kind")
            if name is not None:
                memory_values = dict(values)
                del memory_values["kind"]
                semantic_memory_details[str(name)] = memory_values
            in_semantic_queries = False
            continue
        if line.startswith("semantic-memory"):
            values = parse_key_value_tokens(line[len("semantic-memory"):].strip())
            name = values.get("kind")
            if name is not None:
                memory_values = dict(values)
                del memory_values["kind"]
                semantic_memory[str(name)] = memory_values
            in_semantic_queries = False
            continue
        if in_semantic_queries:
            if line.startswith("    "):
                continue
            if line.startswith("  "):
                row = parse_hotspot_query_row(line)
                if row:
                    semantic_hotspot_queries.append(row)
                continue
            if line.strip():
                in_semantic_queries = False

    return {
        "semantic_metrics": semantic_metrics,
        "semantic_cache_counters": semantic_cache_counters,
        "semantic_cache_sizes": semantic_cache_sizes,
        "semantic_hotspot_summary": semantic_hotspot_summary,
        "semantic_hotspot_queries": semantic_hotspot_queries,
        "semantic_reachability": semantic_reachability,
        "semantic_reachability_queues": semantic_reachability_queues,
        "semantic_memory": semantic_memory,
        "semantic_memory_details": semantic_memory_details,
        "semantic_memory_total": semantic_memory_total,
    }


def run_one(repo_root: Path,
            compiler: str,
            benchmark: Dict[str, object],
            run_index: int,
            build_dir: Path,
            env: Dict[str, str],
            timeout_sec: int,
            poll_interval_sec: float) -> Dict[str, object]:
    source = repo_root / str(benchmark["source"])
    if not source.is_file():
        raise SystemExit(f"missing benchmark source: {source}")

    mode = str(benchmark["mode"])
    out = build_dir / benchmark["name"] / f"run-{run_index}.out"
    out.parent.mkdir(parents=True, exist_ok=True)
    relative_source = str(source.relative_to(repo_root))
    if mode == "compile":
        cmd = [
            compiler,
            "-I",
            "dev/src",
            "-c",
            "-o",
            str(out.with_suffix(".o")),
            relative_source,
        ]
    elif mode == "emit-ast":
        cmd = [
            compiler,
            "--emit-ast",
            "-o",
            str(out),
            relative_source,
        ]
    elif mode == "emit-lowir":
        cmd = [
            compiler,
            "--emit-lowir",
            "-o",
            str(out),
            relative_source,
        ]
    else:
        raise SystemExit(f"unknown benchmark mode for {benchmark['name']}: {mode}")

    started = time.perf_counter()
    proc = subprocess.Popen(
        cmd,
        cwd=str(repo_root),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    timeline: List[Dict[str, object]] = []
    stop_event = threading.Event()
    monitor = threading.Thread(
        target=monitor_process,
        args=(proc.pid, poll_interval_sec, stop_event, timeline),
        daemon=True,
    )
    monitor.start()

    timed_out = False
    try:
        stdout, _ = proc.communicate(timeout=timeout_sec if timeout_sec > 0 else None)
    except subprocess.TimeoutExpired:
        timed_out = True
        proc.kill()
        stdout, _ = proc.communicate()
    finally:
        stop_event.set()
        monitor.join(timeout=max(1.0, poll_interval_sec * 2.0))

    wall_seconds = time.perf_counter() - started
    max_rss_kb = max((int(item["rss_kb"]) for item in timeline), default=0)
    status = "timeout" if timed_out else ("ok" if proc.returncode == 0 else "error")
    if status == "error" and benchmark.get("allow_failure"):
        status = "known-error"

    parsed_counters = parse_counter_output(stdout)
    return {
        "benchmark": benchmark["name"],
        "source": benchmark["source"],
        "run_index": run_index,
        "status": status,
        "returncode": 124 if timed_out else proc.returncode,
        "wall_seconds": wall_seconds,
        "max_rss_kb": max_rss_kb,
        "cmd": cmd,
        "stdout": stdout,
        "timeline_samples": len(timeline),
        "output": str(out),
        **parsed_counters,
    }


def summarize_benchmark(benchmark: Dict[str, object],
                        runs: List[Dict[str, object]]) -> Dict[str, object]:
    ok_runs = [run for run in runs if run["status"] == "ok"]
    measured_runs = [
        run for run in runs
        if run["status"] == "ok" or run["status"] == "known-error"
    ]
    wall_values = [float(run["wall_seconds"]) for run in measured_runs]
    rss_values = [float(run["max_rss_kb"]) for run in measured_runs if run["max_rss_kb"]]
    return {
        "name": benchmark["name"],
        "source": benchmark["source"],
        "mode": benchmark["mode"],
        "tags": benchmark["tags"],
        "runs": len(runs),
        "ok": len(ok_runs),
        "measured": len(measured_runs),
        "known_error": sum(1 for run in runs if run["status"] == "known-error"),
        "timeout": sum(1 for run in runs if run["status"] == "timeout"),
        "error": sum(1 for run in runs if run["status"] == "error"),
        "median_wall_seconds": median(wall_values),
        "min_wall_seconds": min(wall_values) if wall_values else None,
        "max_wall_seconds": max(wall_values) if wall_values else None,
        "median_max_rss_kb": median(rss_values),
        "max_observed_rss_kb": max(rss_values) if rss_values else None,
    }


def load_baseline(path: str) -> Optional[Dict[str, object]]:
    if not path:
        return None
    with Path(path).open("r", encoding="utf-8") as stream:
        return json.load(stream)


def compare_to_baseline(report: Dict[str, object],
                        baseline: Optional[Dict[str, object]]) -> List[Dict[str, object]]:
    if not baseline:
        return []
    baseline_items = {
        item["name"]: item
        for item in baseline.get("summary", [])
    }
    comparisons = []
    for item in report.get("summary", []):
        previous = baseline_items.get(item["name"])
        if not previous:
            continue
        current_wall = item.get("median_wall_seconds")
        previous_wall = previous.get("median_wall_seconds")
        current_rss = item.get("median_max_rss_kb")
        previous_rss = previous.get("median_max_rss_kb")
        wall_delta_pct = None
        rss_delta_pct = None
        if current_wall is not None and previous_wall:
            wall_delta_pct = ((current_wall - previous_wall) / previous_wall) * 100.0
        if current_rss is not None and previous_rss:
            rss_delta_pct = ((current_rss - previous_rss) / previous_rss) * 100.0
        comparisons.append({
            "name": item["name"],
            "baseline_wall_seconds": previous_wall,
            "current_wall_seconds": current_wall,
            "wall_delta_pct": wall_delta_pct,
            "baseline_rss_kb": previous_rss,
            "current_rss_kb": current_rss,
            "rss_delta_pct": rss_delta_pct,
        })
    return comparisons


def format_optional_float(value: object, suffix: str = "") -> str:
    if value is None:
        return "n/a"
    return f"{float(value):.3f}{suffix}"


def metric_value(run: Dict[str, object], key: str) -> int:
    metrics = run.get("semantic_metrics") or {}
    if not isinstance(metrics, dict):
        return 0
    value = metrics.get(key, 0)
    return int(value) if isinstance(value, int) else 0


def cache_size_value(run: Dict[str, object], key: str) -> int:
    sizes = run.get("semantic_cache_sizes") or {}
    if not isinstance(sizes, dict):
        return 0
    value = sizes.get(key, 0)
    return int(value) if isinstance(value, int) else 0


def format_counter_snapshot(run: Dict[str, object]) -> Dict[str, str]:
    resolve = (
        f"calls {metric_value(run, 'resolve-template-argument-calls')}, "
        f"hit/miss {metric_value(run, 'resolve-template-argument-cache-hits')}/"
        f"{metric_value(run, 'resolve-template-argument-cache-misses')}"
    )
    class_templates = (
        f"ref {metric_value(run, 'class-template-reference-requests')}, "
        f"fast {metric_value(run, 'class-template-fast-existing-hits')}/"
        f"{metric_value(run, 'class-template-fast-existing-misses')}, "
        f"hit/create/reset {metric_value(run, 'class-template-hits')}/"
        f"{metric_value(run, 'class-template-creates')}/"
        f"{metric_value(run, 'class-template-resets')}"
    )
    complete = (
        f"calls {metric_value(run, 'complete-class-type-calls')}, "
        f"nonclass-skip {metric_value(run, 'complete-class-type-definitely-not-class-skips')}, "
        f"mat {metric_value(run, 'complete-class-type-materializations')}, "
        f"inprog {metric_value(run, 'complete-class-type-in-progress')}"
    )
    class_info = (
        f"calls {metric_value(run, 'class-info-for-type-calls')}, "
        f"nonclass-skip {metric_value(run, 'class-info-for-type-definitely-not-class-skips')}, "
        f"ptr-hit {metric_value(run, 'class-info-for-type-pointer-cache-hits')}, "
        f"map {metric_value(run, 'class-info-for-type-map-hits')}/"
        f"{metric_value(run, 'class-info-for-type-map-misses')}"
    )
    caches = (
        f"parsed {cache_size_value(run, 'text.parsed_type')}, "
        f"dep {cache_size_value(run, 'dependent.type_resolution')}"
    )
    queries = run.get("semantic_hotspot_queries") or []
    top_query = "n/a"
    if isinstance(queries, list) and queries:
        first = queries[0]
        if isinstance(first, dict):
            top_query = f"{first.get('kind', 'unknown')} x{first.get('count', 0)}"
    return {
        "resolve": resolve,
        "class_templates": class_templates,
        "class_info": class_info,
        "complete": complete,
        "caches": caches,
        "top_query": top_query,
    }


def format_reachability_snapshot(run: Dict[str, object]) -> str:
    reachability = run.get("semantic_reachability") or {}
    if not isinstance(reachability, dict) or not reachability:
        return "n/a"
    return (
        f"classes {reachability.get('classes-reached', 0)}/"
        f"{reachability.get('classes-total', 0)}, "
        f"functions {reachability.get('functions-reached', 0)}/"
        f"{reachability.get('functions-total', 0)}, "
        f"templates {reachability.get('template-instantiations-reached', 0)}/"
        f"{reachability.get('template-instantiations-total', 0)}"
    )


def format_memory_snapshot(run: Dict[str, object]) -> str:
    memory = run.get("semantic_memory") or {}
    details = run.get("semantic_memory_details") or {}
    if not isinstance(memory, dict) or not isinstance(details, dict) or not memory:
        return "n/a"

    def bytes_for(table: Dict[str, object], key: str) -> int:
        item = table.get(key)
        if not isinstance(item, dict):
            return 0
        value = item.get("bytes", 0)
        return int(value) if isinstance(value, int) else 0

    return (
        f"callsem {bytes_for(memory, 'callsem.output')}, "
        f"inline {bytes_for(details, 'callsem.output.inline')}, "
        f"children {bytes_for(details, 'callsem.output.children_storage')}, "
        f"cached {bytes_for(memory, 'callsem.cached_body')}"
    )


def is_counter_output_line(line: str) -> bool:
    stripped = line.strip()
    return (
        stripped.startswith("semantic-metrics") or
        stripped.startswith("semantic-cache") or
        stripped.startswith("semantic-reachability") or
        stripped.startswith("semantic-memory") or
        stripped.startswith("SEMANTIC_HOTSPOT") or
        stripped.startswith("count=") or
        stripped.startswith("frame_count=")
    )


def first_report_output_line(stdout: str) -> str:
    first_counter_line = ""
    for line in stdout.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        if is_counter_output_line(stripped):
            if not first_counter_line:
                first_counter_line = stripped
            continue
        return stripped[:180]
    return first_counter_line[:180] if first_counter_line else "<no output>"


def write_markdown(path: Path, report: Dict[str, object]) -> None:
    lines = []
    lines.append("# Structured-AST Performance Benchmarks")
    lines.append("")
    lines.append("## Run")
    lines.append("")
    lines.append(f"- branch: `{report['branch'] or 'unknown'}`")
    lines.append(f"- head: `{report['head'] or 'unknown'}`")
    lines.append(f"- compiler: `{report['compiler']}`")
    lines.append(f"- repeat: `{report['repeat']}`")
    lines.append(f"- timeout-sec: `{report['timeout_sec']}`")
    lines.append(f"- counters-enabled: `{report['counters_enabled']}`")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append("| Benchmark | Mode | OK | Measured | Median Wall | Median RSS | Source |")
    lines.append("| --- | --- | ---: | ---: | ---: | ---: | --- |")
    for item in report["summary"]:
        wall = format_optional_float(item.get("median_wall_seconds"), "s")
        rss = format_optional_float(item.get("median_max_rss_kb"), " KB")
        lines.append(
            f"| `{item['name']}` | `{item['mode']}` | {item['ok']}/{item['runs']} | "
            f"{item['measured']}/{item['runs']} | {wall} | {rss} | `{item['source']}` |")
    comparisons = report.get("comparisons") or []
    if comparisons:
        lines.append("")
        lines.append("## Baseline Comparison")
        lines.append("")
        lines.append("| Benchmark | Wall Delta | RSS Delta |")
        lines.append("| --- | ---: | ---: |")
        for item in comparisons:
            wall_delta = item.get("wall_delta_pct")
            rss_delta = item.get("rss_delta_pct")
            wall = "n/a" if wall_delta is None else f"{wall_delta:+.2f}%"
            rss = "n/a" if rss_delta is None else f"{rss_delta:+.2f}%"
            lines.append(f"| `{item['name']}` | {wall} | {rss} |")
    counter_runs = [
        run for run in report["runs"]
        if run.get("semantic_metrics") or
           run.get("semantic_cache_counters") or
           run.get("semantic_cache_sizes") or
           run.get("semantic_reachability") or
           run.get("semantic_memory") or
           run.get("semantic_hotspot_summary")
    ]
    if report.get("counters_enabled") and counter_runs:
        lines.append("")
        lines.append("## Counter Snapshot")
        lines.append("")
        lines.append(
            "| Benchmark | Run | Status | Reachability | Resolve Args | Class Templates | Class Info | Complete Class | Cache Sizes | Top Query |")
        lines.append("| --- | ---: | --- | --- | --- | --- | --- | --- | --- | --- |")
        for run in counter_runs:
            snapshot = format_counter_snapshot(run)
            reachability = format_reachability_snapshot(run)
            lines.append(
                f"| `{run['benchmark']}` | {run['run_index']} | `{run['status']}` | "
                f"{reachability} | "
                f"{snapshot['resolve']} | {snapshot['class_templates']} | "
                f"{snapshot['class_info']} | {snapshot['complete']} | "
                f"{snapshot['caches']} | "
                f"`{snapshot['top_query']}` |")
    memory_runs = [
        run for run in report["runs"]
        if run.get("semantic_memory") or run.get("semantic_memory_details")
    ]
    if memory_runs:
        lines.append("")
        lines.append("## Memory Census Snapshot")
        lines.append("")
        lines.append("| Benchmark | Run | Status | CallSem Output |")
        lines.append("| --- | ---: | --- | --- |")
        for run in memory_runs:
            lines.append(
                f"| `{run['benchmark']}` | {run['run_index']} | `{run['status']}` | "
                f"{format_memory_snapshot(run)} |")
    lines.append("")
    lines.append("## Failures")
    lines.append("")
    failures = [
        run for run in report["runs"]
        if run["status"] not in ("ok", "known-error")
    ]
    if not failures:
        lines.append("- none")
    else:
        for run in failures:
            first_line = first_report_output_line(run.get("stdout", ""))
            lines.append(
                f"- `{run['benchmark']}` run {run['run_index']}: "
                f"`{run['status']}` `{first_line}`")
    known_errors = [
        run for run in report["runs"]
        if run["status"] == "known-error"
    ]
    if known_errors:
        lines.append("")
        lines.append("## Known Errors")
        lines.append("")
        for run in known_errors:
            first_line = first_report_output_line(run.get("stdout", ""))
            lines.append(
                f"- `{run['benchmark']}` run {run['run_index']}: "
                f"`{first_line}`")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def build_cppgm(repo_root: Path) -> None:
    subprocess.run(["make", "-C", "dev", "cppgm++"], cwd=str(repo_root), check=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run fixed structured-AST performance benchmarks."
    )
    parser.add_argument("--repo-root", default=str(repo_root_from_script()))
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--repeat", type=int, default=3)
    parser.add_argument("--timeout-sec", type=int, default=180)
    parser.add_argument("--poll-interval-sec", type=float, default=0.2)
    parser.add_argument("--output-prefix",
                        default="/tmp/cppgm-structured-ast-perf")
    parser.add_argument("--build-dir", default="")
    parser.add_argument("--benchmark", action="append", default=[],
                        help="Benchmark name to run. Defaults to all.")
    parser.add_argument("--include-pa34-perf", action="store_true",
                        help="Append focused long-running PA34 compile benchmarks.")
    parser.add_argument("--include-hosted-pa34", action="store_true",
                        help="Deprecated alias for --include-pa34-perf.")
    parser.add_argument("--include-self-compile", action="store_true",
                        help="Append optional frozen self-compile benchmarks.")
    parser.add_argument("--list", action="store_true",
                        help="List benchmarks and exit.")
    parser.add_argument("--skip-build", action="store_true")
    parser.add_argument("--counters", action="store_true",
                        help="Enable semantic stats/hotspot/cache env vars.")
    parser.add_argument("--env", action="append", default=[],
                        help="Extra environment assignment, NAME=VALUE.")
    parser.add_argument("--baseline", default="",
                        help="Previous JSON report to compare against.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    if args.repeat < 1:
        raise SystemExit("--repeat must be >= 1")
    if args.timeout_sec < 0:
        raise SystemExit("--timeout-sec must be >= 0")
    if args.poll_interval_sec <= 0:
        raise SystemExit("--poll-interval-sec must be > 0")

    benchmarks = selected_benchmarks(args.benchmark)
    if not args.benchmark:
        if args.include_pa34_perf or args.include_hosted_pa34:
            benchmarks.extend(FOCUSED_PA34_PERF_BENCHMARKS)
        if args.include_self_compile:
            benchmarks.extend(OPTIONAL_SELF_COMPILE_BENCHMARKS)
    if args.list:
        default_names = {item["name"] for item in DEFAULT_BENCHMARKS}
        pa34_perf_names = {item["name"] for item in FOCUSED_PA34_PERF_BENCHMARKS}
        checkpoint_names = {item["name"] for item in CHECKPOINT_SELF_COMPILE_BENCHMARKS}
        for item in ALL_BENCHMARKS:
            tags = ",".join(item["tags"])
            if item["name"] in default_names:
                group = "default"
            elif item["name"] in pa34_perf_names:
                group = "pa34-perf"
            elif item["name"] in checkpoint_names:
                group = "checkpoint"
            else:
                group = "optional"
            print(f"{item['name']}\t{item['mode']}\t{group}\t{item['source']}\t{tags}")
        return 0

    if not args.skip_build:
        build_cppgm(repo_root)

    compiler = args.compiler
    if compiler.startswith("./"):
        compiler = str((repo_root / compiler[2:]).resolve())
    elif not Path(compiler).is_absolute() and "/" in compiler:
        compiler = str((repo_root / compiler).resolve())

    output_prefix = Path(args.output_prefix)
    output_prefix.parent.mkdir(parents=True, exist_ok=True)
    build_dir = Path(args.build_dir) if args.build_dir else Path(str(output_prefix) + ".build")
    if not build_dir.is_absolute():
        build_dir = repo_root / build_dir
    build_dir.mkdir(parents=True, exist_ok=True)

    env = os.environ.copy()
    env.update(split_env_assignments(args.env))
    if args.counters:
        env["CPPGM_SEMANTIC_STATS"] = "1"
        env["CPPGM_SEMANTIC_HOTSPOT"] = "1"
        env["CPPGM_SEMANTIC_CACHE_STATS"] = "1"

    all_runs = []
    summary = []
    for benchmark in benchmarks:
        runs = []
        print(f"benchmark {benchmark['name']} ({benchmark['source']})", flush=True)
        for run_index in range(1, args.repeat + 1):
            result = run_one(
                repo_root,
                compiler,
                benchmark,
                run_index,
                build_dir,
                env,
                args.timeout_sec,
                args.poll_interval_sec,
            )
            runs.append(result)
            all_runs.append(result)
            print(
                f"  run {run_index}: {result['status']} "
                f"{result['wall_seconds']:.3f}s rss={result['max_rss_kb']} KB",
                flush=True,
            )
        summary.append(summarize_benchmark(benchmark, runs))

    report = {
        "branch": git_branch(repo_root),
        "head": git_head(repo_root),
        "compiler": compiler,
        "repeat": args.repeat,
        "timeout_sec": args.timeout_sec,
        "poll_interval_sec": args.poll_interval_sec,
        "output_prefix": str(output_prefix),
        "build_dir": str(build_dir),
        "counters_enabled": bool(args.counters),
        "benchmarks": benchmarks,
        "summary": summary,
        "runs": all_runs,
    }
    report["comparisons"] = compare_to_baseline(report, load_baseline(args.baseline))

    json_path = Path(str(output_prefix) + ".json")
    md_path = Path(str(output_prefix) + ".md")
    json_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n",
                         encoding="utf-8")
    write_markdown(md_path, report)
    print(f"Wrote {json_path}")
    print(f"Wrote {md_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

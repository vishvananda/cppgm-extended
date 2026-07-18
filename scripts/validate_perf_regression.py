#!/usr/bin/env python3
"""Record and check compiler performance baselines.

The check gates on instruction count and memory. Wall time is recorded for
context, but it is not used to fail a candidate.
"""

import argparse
import datetime as _datetime
import json
from pathlib import Path
import re
import statistics
import subprocess
import sys


DEFAULT_COMMAND = [
    "./dev/cppgm++",
    "-I",
    "benchmarks/self_compile/stable/include",
    "-c",
    "-o",
    "/tmp/cppgm-perf-check.o",
    "benchmarks/self_compile/stable/semantic_overload.cpp",
]

TIME_VALUE_LABEL_RE = re.compile(r"^\s*([0-9]+(?:\.[0-9]+)?)\s+(.+?)\s*$")
TIME_LABEL_VALUE_RE = re.compile(
    r"^\s*([A-Za-z][A-Za-z0-9 _-]*?)\s+([0-9]+(?:\.[0-9]+)?)\s*$"
)

INTEGER_METRICS = {
    "maximum_resident_set_size",
    "average_shared_memory_size",
    "average_unshared_data_size",
    "average_unshared_stack_size",
    "page_reclaims",
    "page_faults",
    "swaps",
    "block_input_operations",
    "block_output_operations",
    "messages_sent",
    "messages_received",
    "signals_received",
    "voluntary_context_switches",
    "involuntary_context_switches",
    "instructions_retired",
    "cycles_elapsed",
    "peak_memory_footprint",
}

MEMORY_METRICS = {
    "maximum_resident_set_size",
    "peak_memory_footprint",
}

FLOAT_METRICS = {
    "real",
    "user",
    "sys",
}

KNOWN_TIME_METRICS = INTEGER_METRICS | FLOAT_METRICS

CHECKS = [
    ("instructions_retired", "instructions", "instruction_tolerance"),
    ("maximum_resident_set_size", "max rss", "rss_tolerance"),
    ("peak_memory_footprint", "peak footprint", "footprint_tolerance"),
]


def normalize_metric_name(label):
    return label.strip().lower().replace(" ", "_").replace("-", "_")


def parse_time_metrics(stderr):
    metrics = {}
    non_time_lines = []
    for line in stderr.splitlines():
        match = TIME_VALUE_LABEL_RE.match(line)
        if match:
            raw_value, label = match.groups()
        else:
            match = TIME_LABEL_VALUE_RE.match(line)
            if not match:
                non_time_lines.append(line)
                continue
            label, raw_value = match.groups()
            key = normalize_metric_name(label)
            if key not in KNOWN_TIME_METRICS:
                non_time_lines.append(line)
                continue

        key = normalize_metric_name(label)
        if key not in KNOWN_TIME_METRICS:
            non_time_lines.append(line)
            continue

        if key in INTEGER_METRICS:
            value = int(float(raw_value))
        else:
            value = float(raw_value)
        metrics[key] = value

    return metrics, "\n".join(non_time_lines).strip()


def repo_head(repo_root):
    try:
        proc = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=str(repo_root),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return None
    return proc.stdout.strip()


def clean_command(command):
    cleaned = list(command)
    if cleaned and cleaned[0] == "--":
        cleaned = cleaned[1:]
    return cleaned or list(DEFAULT_COMMAND)


def normalized_workload_command(command):
    normalized = []
    skip_output = False
    for arg in command:
        if skip_output:
            normalized.append("<output>")
            skip_output = False
            continue
        if arg == "-o":
            normalized.append(arg)
            skip_output = True
            continue
        if arg.startswith("-o") and len(arg) > 2:
            normalized.append("-o<output>")
            continue
        normalized.append(arg)
    return normalized


def run_once(time_binary, repo_root, command, timeout_sec):
    timed_command = [time_binary, "-lp"] + command
    try:
        proc = subprocess.run(
            timed_command,
            cwd=str(repo_root),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout_sec if timeout_sec > 0 else None,
        )
    except subprocess.TimeoutExpired as exc:
        print("Command timed out after %s seconds:" % timeout_sec, file=sys.stderr)
        print(" ".join(command), file=sys.stderr)
        if exc.stdout:
            print(exc.stdout, file=sys.stderr)
        if exc.stderr:
            print(exc.stderr, file=sys.stderr)
        raise SystemExit(2)

    metrics, command_stderr = parse_time_metrics(proc.stderr)
    if proc.returncode != 0:
        print("Command failed with exit code %d:" % proc.returncode, file=sys.stderr)
        print(" ".join(command), file=sys.stderr)
        if proc.stdout:
            print(proc.stdout, file=sys.stderr)
        if proc.stderr:
            print(proc.stderr, file=sys.stderr)
        raise SystemExit(proc.returncode)

    return {
        "metrics": metrics,
        "stdout": proc.stdout.strip(),
        "stderr": command_stderr,
    }


def collect_runs(args, command):
    runs = []
    for index in range(args.runs):
        print("run %d/%d..." % (index + 1, args.runs), flush=True)
        runs.append(
            run_once(
                args.time_binary,
                Path(args.repo_root).resolve(),
                command,
                args.timeout_sec,
            )
        )
    return runs


def summarize_runs(runs):
    keys = sorted({key for run in runs for key in run["metrics"].keys()})
    summary = {}
    for key in keys:
        values = [run["metrics"][key] for run in runs if key in run["metrics"]]
        if not values:
            continue
        summary[key] = {
            "median": statistics.median(values),
            "min": min(values),
            "max": max(values),
        }
    return summary


def make_report(args, command, runs):
    repo_root = Path(args.repo_root).resolve()
    return {
        "created_at": _datetime.datetime.now(
            _datetime.timezone.utc
        ).isoformat(),
        "repo_root": str(repo_root),
        "head": repo_head(repo_root),
        "command": command,
        "runs": runs,
        "summary": summarize_runs(runs),
    }


def write_json(path, data):
    output_path = Path(path)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")


def load_json(path):
    with Path(path).open() as handle:
        return json.load(handle)


def format_bytes(value):
    value = float(value)
    units = ["B", "KiB", "MiB", "GiB"]
    unit = units[0]
    for unit in units:
        if abs(value) < 1024.0 or unit == units[-1]:
            break
        value /= 1024.0
    return "%.2f %s" % (value, unit)


def format_metric(key, value):
    if value is None:
        return "missing"
    if key in MEMORY_METRICS:
        return "%s (%s)" % (format_bytes(value), f"{int(value):,}")
    if key in INTEGER_METRICS:
        return f"{int(value):,}"
    return "%.3f" % float(value)


def print_record_summary(report):
    print("")
    print("Recorded baseline")
    print("  head: %s" % (report.get("head") or "unknown"))
    print("  command: %s" % " ".join(report["command"]))
    for key in [
        "instructions_retired",
        "maximum_resident_set_size",
        "peak_memory_footprint",
        "real",
        "user",
        "sys",
    ]:
        if key in report["summary"]:
            value = report["summary"][key]["median"]
            print("  %-28s %s" % (key + ":", format_metric(key, value)))


def compare_reports(baseline, candidate, args):
    baseline_summary = baseline.get("summary", {})
    candidate_summary = candidate.get("summary", {})
    failures = []
    baseline_command = normalized_workload_command(baseline.get("command", []))
    candidate_command = normalized_workload_command(candidate.get("command", []))
    if baseline_command != candidate_command:
        failures.append(
            "benchmark workload command differs: baseline=%s candidate=%s"
            % (" ".join(baseline_command), " ".join(candidate_command))
        )
    rows = []

    for key, label, tolerance_attr in CHECKS:
        tolerance = getattr(args, tolerance_attr)
        baseline_value = baseline_summary.get(key, {}).get("median")
        candidate_value = candidate_summary.get(key, {}).get("median")
        if baseline_value is None or candidate_value is None:
            failures.append("%s is missing from baseline or candidate" % key)
            rows.append((label, key, baseline_value, candidate_value, tolerance, None, False))
            continue

        limit = float(baseline_value) * (1.0 + tolerance)
        passed = float(candidate_value) <= limit
        delta = (float(candidate_value) / float(baseline_value)) - 1.0
        if not passed:
            failures.append(
                "%s increased by %.2f%%; tolerance is %.2f%%"
                % (key, delta * 100.0, tolerance * 100.0)
            )
        rows.append((label, key, baseline_value, candidate_value, tolerance, delta, passed))

    print("")
    print("Performance check")
    print("  baseline head: %s" % (baseline.get("head") or "unknown"))
    print("  candidate head: %s" % (candidate.get("head") or "unknown"))
    print("")
    print("%-16s %-28s %-28s %-10s %-10s" % ("metric", "baseline", "candidate", "delta", "status"))
    for label, key, baseline_value, candidate_value, tolerance, delta, passed in rows:
        delta_text = "missing" if delta is None else "%+.2f%%" % (delta * 100.0)
        status = "PASS" if passed else "FAIL"
        status += " (tol %.2f%%)" % (tolerance * 100.0)
        print(
            "%-16s %-28s %-28s %-10s %-10s"
            % (
                label,
                format_metric(key, baseline_value),
                format_metric(key, candidate_value),
                delta_text,
                status,
            )
        )

    for key in ["real", "user", "sys", "cycles_elapsed"]:
        baseline_value = baseline_summary.get(key, {}).get("median")
        candidate_value = candidate_summary.get(key, {}).get("median")
        if baseline_value is None or candidate_value is None:
            continue
        delta = (float(candidate_value) / float(baseline_value)) - 1.0
        print(
            "  info %-20s %s -> %s (%+.2f%%)"
            % (
                key + ":",
                format_metric(key, baseline_value),
                format_metric(key, candidate_value),
                delta * 100.0,
            )
        )

    return failures


def command_record(args):
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")
    command = clean_command(args.command)
    runs = collect_runs(args, command)
    report = make_report(args, command, runs)
    write_json(args.baseline, report)
    print_record_summary(report)
    print("  wrote: %s" % args.baseline)
    return 0


def command_check(args):
    if args.runs < 1:
        raise SystemExit("--runs must be at least 1")
    command = clean_command(args.command)
    baseline = load_json(args.baseline)
    runs = collect_runs(args, command)
    candidate = make_report(args, command, runs)
    failures = compare_reports(baseline, candidate, args)
    if args.report:
        payload = {
            "baseline": baseline,
            "candidate": candidate,
            "failures": failures,
        }
        write_json(args.report, payload)
        print("  wrote report: %s" % args.report)

    if failures:
        print("")
        for failure in failures:
            print("FAIL: %s" % failure)
        return 1

    print("")
    print("PASS: candidate is within instruction and memory tolerances")
    return 0


def add_common_args(parser, runs_default):
    parser.add_argument("--repo-root", default=Path(__file__).resolve().parents[1])
    parser.add_argument("--runs", type=int, default=runs_default)
    parser.add_argument("--timeout-sec", type=int, default=0)
    parser.add_argument("--time-binary", default="/usr/bin/time")
    parser.add_argument("command", nargs=argparse.REMAINDER)


def build_parser():
    parser = argparse.ArgumentParser(
        description="Record or check cppgm performance baselines."
    )
    subparsers = parser.add_subparsers(dest="mode", required=True)

    record = subparsers.add_parser("record", help="record a baseline JSON file")
    record.add_argument("--baseline", required=True)
    add_common_args(record, runs_default=3)
    record.set_defaults(func=command_record)

    check = subparsers.add_parser("check", help="check against a baseline JSON file")
    check.add_argument("--baseline", required=True)
    check.add_argument("--report")
    check.add_argument("--instruction-tolerance", type=float, default=0.01)
    check.add_argument("--rss-tolerance", type=float, default=0.03)
    check.add_argument("--footprint-tolerance", type=float, default=0.03)
    add_common_args(check, runs_default=1)
    check.set_defaults(func=command_check)

    return parser


def main(argv):
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))

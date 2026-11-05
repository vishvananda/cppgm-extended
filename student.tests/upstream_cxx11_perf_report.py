#!/usr/bin/env python3
"""Assess paired perf counter evidence attached to run_ab_compile_benchmark runs.

Counter thresholds are declared before the experiment in performance-protocol.json.
Wall timing remains reported even when load makes its conclusion inconclusive.
"""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import statistics
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from run_ab_compile_benchmark import paired_ratios


def counters(path):
    values = {}
    for fields in csv.reader(path.read_text().splitlines(), delimiter=";"):
        if len(fields) < 5 or fields[0].startswith("#"):
            continue
        name = fields[2].removesuffix(":u")
        try:
            value, percent = float(fields[0]), float(fields[4])
        except ValueError as error:
            raise ValueError(f"{path}: unavailable counter {name}") from error
        if percent < 99:
            raise ValueError(f"{path}: multiplexed {name}: {percent}%")
        values[name] = value
    required = {"instructions", "cycles", "task-clock", "context-switches", "cpu-migrations", "page-faults"}
    if not required.issubset(values) or values["instructions"] <= 0:
        raise ValueError(f"{path}: missing or invalid counters")
    return values


def interval(ratios):
    if len(ratios) != 10 or any(x <= 0 for x in ratios):
        raise ValueError("expected ten complete paired blocks with positive ratios")
    logs = [math.log(x) for x in ratios]
    mean = statistics.mean(logs)
    half = 2.262157 * statistics.stdev(logs) / math.sqrt(10)
    return dict(ratios=ratios, geometric_mean=math.exp(mean), lower_95=math.exp(mean-half),
                upper_95=math.exp(mean+half), multiplicative_halfwidth=math.exp(half)-1)


def assess(path, phase, calibration_tolerance=0):
    report = json.loads(path.read_text())
    runs = report["runs"]
    if len(runs) != 40 or any(r["status"] != "ok" for r in runs):
        raise ValueError(f"{path}: incomplete or failed experiment")
    hashes = {r.get("output_sha256") for r in runs}
    if len(hashes) != 1 or None in hashes:
        raise ValueError(f"{path}: unequal or absent objects")
    for run in runs:
        args = run["command"]
        output = Path(args[args.index("-o") + 1])
        run.update(counters(Path(str(output) + ".perf")))
    metrics = {key: interval(paired_ratios(runs, key)) for key in
               ["instructions", "cycles", "wall_seconds", "user_seconds", "max_rss_kib"]}
    errors = []
    if phase == "aa":
        instructions = metrics["instructions"]
        if instructions["multiplicative_halfwidth"] > .005:
            errors.append("instruction calibration noise exceeds 0.5%")
        if calibration_tolerance:
            if (instructions["lower_95"] < 1 - calibration_tolerance or
                    instructions["upper_95"] > 1 + calibration_tolerance):
                errors.append("instruction A/A interval exceeds predeclared equivalence band")
        elif not instructions["lower_95"] <= 1 <= instructions["upper_95"]:
            errors.append("instruction A/A interval excludes 1; investigate bias")
    else:
        if metrics["instructions"]["upper_95"] > 1.01:
            errors.append("instruction upper confidence bound exceeds 1% regression gate")
    if metrics["max_rss_kib"]["upper_95"] > 1.03:
        errors.append("memory upper confidence bound exceeds 3% regression gate")
    return dict(path=str(path), phase=phase, metrics=metrics, errors=errors,
                status="pass" if not errors else "investigate", object_sha256=next(iter(hashes)))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--phase", choices=["aa", "ab"], required=True)
    parser.add_argument("--expected-workloads", type=int, default=3)
    parser.add_argument("--calibration-tolerance", type=float, default=0,
                        help="A/A equivalence half-band as a ratio; zero retains the original equality check")
    args = parser.parse_args()
    binary_manifest = args.directory / "underlying-binaries.json"
    for path, digest in json.loads(binary_manifest.read_text()).items():
        if hashlib.sha256(Path(path).read_bytes()).hexdigest() != digest:
            raise ValueError(f"compiler changed: {path}")
    reports = sorted(args.directory.glob("*-" + args.phase + ".json"))
    if len(reports) != args.expected_workloads:
        raise ValueError(f"expected {args.expected_workloads} predeclared workloads")
    results = [assess(path, args.phase, args.calibration_tolerance) for path in reports]
    destination = args.directory / (args.phase + "-assessment.json")
    destination.write_text(json.dumps(results, indent=2) + "\n")
    for result in results:
        print(Path(result["path"]).name, result["status"], result["metrics"]["instructions"], result["errors"])
    return int(any(result["errors"] for result in results))


if __name__ == "__main__":
    sys.exit(main())

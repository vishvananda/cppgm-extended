#!/usr/bin/env python3
"""Run the tracked Boost B2 test-suite inventory and summarize results."""

import argparse
import csv
import datetime as _datetime
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import sys
import time
from typing import Dict, Iterable, List, Optional


DEFAULT_BOOST_ROOT = Path("/Users/vishvananda/boost_1_91_0")
DEFAULT_INVENTORY = Path("docs/boost-b2-suite-status-20260511.md")


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def git_short_sha(root: Path) -> str:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short=9", "HEAD"],
            cwd=str(root),
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return "unknown"
    return result.stdout.strip() or "unknown"


def parse_inventory(path: Path) -> List[Dict[str, object]]:
    rows: List[Dict[str, object]] = []
    row_re = re.compile(
        r"^\|\s*(\d+)\s*\|\s*`([^`]+)`\s*\|\s*([^|]+?)\s*\|"
    )
    for line in path.read_text().splitlines():
        match = row_re.match(line)
        if not match:
            continue
        rows.append(
            {
                "index": int(match.group(1)),
                "suite": match.group(2).strip(),
                "previous_status": match.group(3).strip(),
            }
        )
    if not rows:
        raise SystemExit(f"no suite rows found in {path}")
    return rows


def parse_suite_selector(value: str) -> object:
    value = value.strip()
    if not value:
        raise argparse.ArgumentTypeError("empty suite selector")
    try:
        return int(value)
    except ValueError:
        return value


def choose_suites(
    inventory: List[Dict[str, object]],
    selectors: Iterable[object],
    statuses: Iterable[str],
    start_at: Optional[object],
    limit: Optional[int],
) -> List[Dict[str, object]]:
    selected = list(inventory)

    selector_list = list(selectors)
    if selector_list:
        by_index = {row["index"]: row for row in inventory}
        by_suite = {row["suite"]: row for row in inventory}
        selected = []
        for selector in selector_list:
            row = by_index.get(selector) if isinstance(selector, int) else by_suite.get(selector)
            if row is None:
                raise SystemExit(f"unknown suite selector: {selector}")
            selected.append(row)

    wanted_statuses = {status.strip() for status in statuses if status.strip()}
    if wanted_statuses:
        selected = [
            row
            for row in selected
            if str(row["previous_status"]) in wanted_statuses
        ]

    if start_at is not None:
        start_index = None
        for i, row in enumerate(selected):
            if row["index"] == start_at or row["suite"] == start_at:
                start_index = i
                break
        if start_index is None:
            raise SystemExit(f"start suite not in selected set: {start_at}")
        selected = selected[start_index:]

    if limit is not None:
        selected = selected[:limit]

    return selected


def safe_suite_log_name(suite: str) -> str:
    return suite.strip("/").replace("/", "__").replace(":", "_") + ".log"


def classify_result(returncode: int, timed_out: bool, log_text: str) -> str:
    if timed_out:
        return "timeout"
    if returncode == 0:
        return "passing"

    lower = log_text.lower()
    setup_markers = [
        "unable to load boost.build",
        "could not find",
        "no such file or directory",
        "don't know how to make",
        "unknown toolset",
    ]
    if any(marker in lower for marker in setup_markers):
        return "setup fail"

    if re.search(r"\bupdated\s+\d+\s+target", lower):
        return "mixed"
    return "failing"


def extract_detail(log_text: str, status: str) -> str:
    if status == "passing":
        for line in reversed(log_text.splitlines()):
            stripped = line.strip()
            if "updated" in stripped and "target" in stripped:
                return stripped
        return "B2 completed successfully"

    interesting: List[str] = []
    target_re = re.compile(r"^\s*(?:\.\.\.)?failed\s+([^ ]+)")
    for line in log_text.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        match = target_re.match(stripped)
        if match:
            interesting.append(match.group(1))
            continue
        if stripped.startswith("ERROR:") or stripped.startswith("error:"):
            interesting.append(stripped[:180])
        if len(interesting) >= 20:
            break
    return ", ".join(interesting[:20]) if interesting else status


def run_suite(
    boost_root: Path,
    suite: str,
    jobs: int,
    timeout: Optional[int],
    log_path: Path,
) -> Dict[str, object]:
    env = os.environ.copy()
    env["JOBS"] = str(jobs)
    command = ["./run-cppgm-b2.sh", "-a", suite]
    start = time.monotonic()
    timed_out = False
    returncode = 0
    with log_path.open("w", encoding="utf-8", errors="replace") as log:
        log.write(f"$ cd {boost_root}\n")
        log.write(f"$ JOBS={jobs} {' '.join(command)}\n\n")
        log.flush()
        proc = subprocess.Popen(
            command,
            cwd=str(boost_root),
            env=env,
            text=True,
            stdout=log,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        try:
            returncode = proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            returncode = 124
            log.write(f"\nTIMEOUT after {timeout} seconds\n")
            log.flush()
            try:
                os.killpg(proc.pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                try:
                    os.killpg(proc.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                proc.wait()
    elapsed = time.monotonic() - start
    log_text = log_path.read_text(encoding="utf-8", errors="replace")
    status = classify_result(returncode, timed_out, log_text)
    return {
        "status": status,
        "returncode": returncode,
        "duration_seconds": round(elapsed, 1),
        "detail": extract_detail(log_text, status),
        "log": str(log_path),
    }


def write_csv(path: Path, rows: List[Dict[str, object]]) -> None:
    fieldnames = [
        "index",
        "suite",
        "previous_status",
        "status",
        "returncode",
        "duration_seconds",
        "detail",
        "log",
    ]
    with path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow({key: row.get(key, "") for key in fieldnames})


def write_markdown(path: Path, rows: List[Dict[str, object]], metadata: Dict[str, object]) -> None:
    counts: Dict[str, int] = {}
    for row in rows:
        counts[str(row["status"])] = counts.get(str(row["status"]), 0) + 1

    with path.open("w", encoding="utf-8") as f:
        f.write("# Boost B2 Suite Survey\n\n")
        for key in ["generated", "worktree", "boost_root", "jobs", "timeout", "head"]:
            f.write(f"- {key}: `{metadata[key]}`\n")
        f.write(f"- completed suites: {len(rows)}\n")
        for status in sorted(counts):
            f.write(f"- {status}: {counts[status]}\n")
        f.write("\n| # | Suite | Previous | Current | Seconds | Detail | Log |\n")
        f.write("|---|---|---|---|---:|---|---|\n")
        for row in rows:
            detail = str(row.get("detail", "")).replace("|", "\\|")
            f.write(
                f"| {row['index']} | `{row['suite']}` | {row['previous_status']} | "
                f"{row['status']} | {row.get('duration_seconds', '')} | "
                f"{detail} | `{row.get('log', '')}` |\n"
            )


def default_output_dir(root: Path, jobs: int) -> Path:
    stamp = _datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
    return Path(f"/tmp/boost-suite-survey-{stamp}-j{jobs}-{git_short_sha(root)}")


def main(argv: Optional[List[str]] = None) -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(
        description="Run Boost B2 suites from the tracked suite inventory."
    )
    parser.add_argument(
        "--inventory",
        type=Path,
        default=root / DEFAULT_INVENTORY,
        help="Markdown suite inventory to read.",
    )
    parser.add_argument(
        "--boost-root",
        type=Path,
        default=Path(os.environ.get("BOOST_ROOT", str(DEFAULT_BOOST_ROOT))),
        help="Boost checkout containing run-cppgm-b2.sh.",
    )
    parser.add_argument("--jobs", type=int, default=int(os.environ.get("JOBS", "12")))
    parser.add_argument("--timeout", type=int, default=None, help="Per-suite timeout in seconds.")
    parser.add_argument("--output-dir", type=Path, default=None)
    parser.add_argument(
        "--suite",
        action="append",
        default=[],
        type=parse_suite_selector,
        help="Suite path or inventory number. Can be repeated.",
    )
    parser.add_argument("--start-at", type=parse_suite_selector, default=None)
    parser.add_argument("--limit", type=int, default=None)
    parser.add_argument(
        "--previous-status",
        action="append",
        default=[],
        help="Only run suites with this previous inventory status. Can be repeated.",
    )
    parser.add_argument("--stop-on-fail", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args(argv)

    inventory = parse_inventory(args.inventory)
    selected = choose_suites(
        inventory,
        args.suite,
        args.previous_status,
        args.start_at,
        args.limit,
    )
    if args.dry_run:
        for row in selected:
            print(f"{row['index']:>3} {row['suite']} [{row['previous_status']}]")
        print(f"selected {len(selected)} / {len(inventory)} suites")
        return 0

    boost_root = args.boost_root.resolve()
    runner = boost_root / "run-cppgm-b2.sh"
    if not runner.exists():
        raise SystemExit(f"missing Boost runner: {runner}")

    output_dir = args.output_dir or default_output_dir(root, args.jobs)
    output_dir.mkdir(parents=True, exist_ok=True)
    rows: List[Dict[str, object]] = []
    metadata = {
        "generated": _datetime.datetime.now().isoformat(timespec="seconds"),
        "worktree": str(root),
        "boost_root": str(boost_root),
        "jobs": args.jobs,
        "timeout": args.timeout if args.timeout is not None else "none",
        "head": git_short_sha(root),
    }

    for ordinal, row in enumerate(selected, 1):
        suite = str(row["suite"])
        log_path = output_dir / safe_suite_log_name(suite)
        print(f"[{ordinal}/{len(selected)}] {suite}", flush=True)
        result = run_suite(boost_root, suite, args.jobs, args.timeout, log_path)
        merged = dict(row)
        merged.update(result)
        rows.append(merged)
        print(
            f"  {merged['status']} rc={merged['returncode']} "
            f"{merged['duration_seconds']}s log={merged['log']}",
            flush=True,
        )
        write_csv(output_dir / "results.csv", rows)
        (output_dir / "results.json").write_text(
            json.dumps({"metadata": metadata, "results": rows}, indent=2) + "\n",
            encoding="utf-8",
        )
        write_markdown(output_dir / "summary.md", rows, metadata)
        if args.stop_on_fail and merged["status"] != "passing":
            break

    counts: Dict[str, int] = {}
    for row in rows:
        counts[str(row["status"])] = counts.get(str(row["status"]), 0) + 1
    print(f"wrote {output_dir}")
    for status in sorted(counts):
        print(f"{status}: {counts[status]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

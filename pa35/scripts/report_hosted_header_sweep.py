#!/usr/bin/env python3

import argparse
import concurrent.futures
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
from collections import defaultdict


DEFAULT_STD = "gnu++11"
SKIP_SUFFIXES = {
    ".in",
    ".modulemap",
    ".modules",
    ".modules.json",
    ".txt",
    ".py",
    ".sh",
    ".plist",
    ".dat",
    ".syms",
}
SKIP_BASENAMES = {
    "CMakeLists.txt",
    "libcxx.imp",
    "module.modulemap",
}


def detect_jobs():
    env_jobs = os.environ.get("CPPGM_TEST_JOBS")
    if env_jobs and env_jobs.isdigit() and int(env_jobs) > 0:
        return int(env_jobs)
    return max(1, os.cpu_count() or 1)


def run_command(command, env=None):
    proc = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        env=env,
    )
    return proc.returncode, proc.stdout


def compiler_search_paths(host_cxx):
    code, output = run_command([host_cxx, "-E", "-x", "c++", "-", "-v"], env=os.environ.copy())
    if code not in (0, 1):
        raise RuntimeError("unable to probe host compiler include search paths")
    in_block = False
    paths = []
    for line in output.splitlines():
        if "#include <...> search starts here:" in line:
            in_block = True
            continue
        if "End of search list." in line:
            break
        if in_block:
            text = line.strip()
            if text:
                paths.append(text)
    return paths


def libcxx_candidate_roots(host_cxx):
    candidates = set()
    for raw_path in compiler_search_paths(host_cxx):
        if "include/c++/v1" not in raw_path:
            continue
        path = pathlib.Path(raw_path)
        candidates.add(path)
        for parent in path.parents:
            sdk_dir = parent / "SDKs"
            if not sdk_dir.is_dir():
                continue
            for candidate in sdk_dir.glob("*/usr/include/c++/v1"):
                candidates.add(candidate)
    return sorted(candidate for candidate in candidates if candidate.is_dir())


def discover_libcxx_root(host_cxx, recursive, include_internal):
    candidates = libcxx_candidate_roots(host_cxx)
    if not candidates:
        raise RuntimeError("unable to locate a libc++ include root; pass --header-root explicitly")
    ranked = []
    for candidate in candidates:
        ranked.append((len(enumerate_headers(candidate, recursive, include_internal)), candidate))
    ranked.sort(key=lambda item: (-item[0], str(item[1])))
    return ranked[0][1]


def is_header_candidate(path):
    if not path.is_file():
        return False
    if path.name in SKIP_BASENAMES:
        return False
    if path.suffix in SKIP_SUFFIXES:
        return False
    if path.suffix and path.suffix != ".h":
        return False
    return True


def enumerate_headers(root, recursive, include_internal):
    headers = []
    iterator = root.rglob("*") if recursive else root.iterdir()
    for path in iterator:
        if is_header_candidate(path):
            relative = path.relative_to(root).as_posix()
            if not include_internal and relative.split("/", 1)[0].startswith("__"):
                continue
            headers.append(path)
    return sorted(headers)


def safe_header_stem(header):
    digest = hashlib.sha1(header.encode("utf-8")).hexdigest()[:12]
    return digest + "-" + re.sub(r"[^A-Za-z0-9_.-]+", "_", header)


def normalize_diag(text):
    lines = [line.rstrip() for line in text.splitlines() if line.strip()]
    if not lines:
        return "unknown failure"
    normalized = []
    for line in lines:
        line = re.sub(r"/tmp/[A-Za-z0-9_.\-/]+", "<tmp>", line)
        line = re.sub(r"/[A-Za-z0-9_.\-/]+", "<path>", line)
        line = re.sub(r":\d+(:\d+)?", ":<n>", line)
        line = re.sub(r" \[template-scope-depth=.*", "", line)
        line = re.sub(r" \[(function|is_method|body_parent_scope|body_parent_bindings).*", "", line)
        line = re.sub(r" at <path>.*", "", line)
        normalized.append(line)
    context = []
    for idx, line in enumerate(normalized):
        if line == "Diagnostic context:":
            context = [entry.strip() for entry in normalized[idx + 1 : idx + 4] if entry.strip()]
            break
    first = normalized[0]
    if context:
        return first + " | " + context[0]
    return first


def render_report(args, header_root, summary, actionable_clusters, skipped_host, failed_setup):
    def clipped(lines):
        result = []
        for line in lines:
            if len(line) > 240:
                result.append(line[:237] + "...")
            else:
                result.append(line)
        return result

    lines = []
    lines.append("# Hosted Header Sweep Report")
    lines.append("")
    lines.append(f"- Compiler: `{args.compiler}`")
    lines.append(f"- Host compiler: `{args.host_cxx}`")
    lines.append(f"- Header root: `{header_root}`")
    lines.append(f"- Mode: `{'recursive' if args.recursive else 'top-level'}`")
    lines.append(f"- Internal headers included: `{'yes' if args.include_internal else 'no'}`")
    lines.append(f"- Standard: `{args.std}`")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- Total discovered headers: `{summary['total']}`")
    lines.append(f"- Host self-contained headers: `{summary['host_ok']}`")
    lines.append(f"- `cppgm++` passes: `{summary['cppgm_ok']}`")
    lines.append(f"- Actionable host-pass / cppgm-fail headers: `{summary['actionable']}`")
    lines.append(f"- Skipped because host compiler also failed: `{len(skipped_host)}`")
    lines.append(f"- Setup failures: `{len(failed_setup)}`")
    lines.append("")
    lines.append("## Failure Clusters")
    lines.append("")
    if not actionable_clusters:
        lines.append("No actionable `cppgm++` failures were found in this sweep.")
        lines.append("")
    else:
        for fingerprint, entries in sorted(
            actionable_clusters.items(),
            key=lambda item: (-len(item[1]), item[0]),
        ):
            sample = entries[0]
            lines.append(f"### `{len(entries)}` x `{fingerprint}`")
            lines.append("")
            lines.append(f"- Representative header: `<{sample['header']}>`")
            lines.append("- Sample diagnostic:")
            lines.append("```text")
            lines.extend(clipped(sample["cppgm_diag"].rstrip().splitlines()[:20] or ["<empty>"]))
            lines.append("```")
            lines.append("- Additional headers:")
            for header in sorted(entry["header"] for entry in entries[:15]):
                lines.append(f"  - `<{header}>`")
            if len(entries) > 15:
                lines.append(f"  - ... `{len(entries) - 15}` more")
            lines.append("")
    lines.append("## Host-Skipped Headers")
    lines.append("")
    if not skipped_host:
        lines.append("None.")
        lines.append("")
    else:
        for entry in skipped_host[:30]:
            lines.append(f"- `<{entry['header']}>`")
        if len(skipped_host) > 30:
            lines.append(f"- ... `{len(skipped_host) - 30}` more")
        lines.append("")
    lines.append("## Setup Failures")
    lines.append("")
    if not failed_setup:
        lines.append("None.")
        lines.append("")
    else:
        for entry in failed_setup:
            lines.append(f"- `<{entry['header']}>`: `{entry['error']}`")
        lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Batch hosted-header sweep for cppgm++")
    default_host_cxx = (
        os.environ.get("CPPGM_HOST_CXX")
        or os.environ.get("CXX")
        or "c++"
    )
    parser.add_argument("--compiler", default="../dev/cppgm++")
    parser.add_argument("--host-cxx", default=default_host_cxx)
    parser.add_argument("--header-root")
    parser.add_argument("--output", default="testout/libcxx-header-sweep.md")
    parser.add_argument("--json-output", default="testout/libcxx-header-sweep.json")
    parser.add_argument("--work-dir", default="testout/libcxx-header-sweep-work")
    parser.add_argument("--jobs", type=int, default=detect_jobs())
    parser.add_argument("--limit", type=int)
    parser.add_argument("--recursive", action="store_true")
    parser.add_argument("--include-internal", action="store_true")
    parser.add_argument("--std", default=DEFAULT_STD)
    args = parser.parse_args()

    compiler = shutil.which(args.compiler) if os.path.sep not in args.compiler else args.compiler
    if not compiler or not pathlib.Path(compiler).exists():
        raise SystemExit(f"compiler not found: {args.compiler}")
    host_cxx = shutil.which(args.host_cxx) if os.path.sep not in args.host_cxx else args.host_cxx
    if not host_cxx or not pathlib.Path(host_cxx).exists():
        raise SystemExit(f"host compiler not found: {args.host_cxx}")
    args.compiler = compiler
    args.host_cxx = host_cxx

    header_root = (
        pathlib.Path(args.header_root)
        if args.header_root
        else discover_libcxx_root(host_cxx, args.recursive, args.include_internal)
    )
    headers = [
        path.relative_to(header_root).as_posix()
        for path in enumerate_headers(header_root, args.recursive, args.include_internal)
    ]
    if args.limit:
        headers = headers[: args.limit]

    work_dir = pathlib.Path(args.work_dir)
    tu_dir = work_dir / "tu"
    log_dir = work_dir / "logs"
    shutil.rmtree(work_dir, ignore_errors=True)
    tu_dir.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)

    results = []
    skipped_host = []
    failed_setup = []
    actionable_clusters = defaultdict(list)
    summary = {
        "total": len(headers),
        "host_ok": 0,
        "cppgm_ok": 0,
        "actionable": 0,
    }

    def worker(header):
        stem = safe_header_stem(header)
        tu_path = tu_dir / f"{stem}.cpp"
        host_obj = work_dir / f"{stem}.host.o"
        cppgm_obj = work_dir / f"{stem}.cppgm.o"
        tu_path.write_text(f"#include <{header}>\nint main() {{ return 0; }}\n")

        include_flags = ["-I", str(header_root)] if args.header_root else []
        host_cmd = [host_cxx, f"-std={args.std}", "-c", "-o", str(host_obj), str(tu_path)] + include_flags
        host_rc, host_out = run_command(host_cmd)
        if host_rc != 0:
            return {
                "header": header,
                "host_ok": False,
                "host_diag": host_out,
            }

        env = os.environ.copy()
        env["CPPGM_HOST_CXX"] = host_cxx
        cppgm_cmd = [compiler, "-c"] + include_flags + ["-o", str(cppgm_obj), str(tu_path)]
        cppgm_rc, cppgm_out = run_command(cppgm_cmd, env=env)
        return {
            "header": header,
            "host_ok": True,
            "host_diag": host_out,
            "cppgm_ok": cppgm_rc == 0,
            "cppgm_diag": cppgm_out,
            "fingerprint": normalize_diag(cppgm_out) if cppgm_rc != 0 else "",
        }

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, args.jobs)) as executor:
        future_map = {executor.submit(worker, header): header for header in headers}
        for future in concurrent.futures.as_completed(future_map):
            header = future_map[future]
            try:
                result = future.result()
            except Exception as exc:  # pragma: no cover - failure reporting path
                failed_setup.append({"header": header, "error": str(exc)})
                continue
            results.append(result)
            if not result["host_ok"]:
                skipped_host.append(result)
                continue
            summary["host_ok"] += 1
            if result["cppgm_ok"]:
                summary["cppgm_ok"] += 1
                continue
            summary["actionable"] += 1
            actionable_clusters[result["fingerprint"]].append(result)

    pathlib.Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    pathlib.Path(args.output).write_text(
        render_report(args, header_root, summary, actionable_clusters, skipped_host, failed_setup)
        + "\n"
    )
    pathlib.Path(args.json_output).parent.mkdir(parents=True, exist_ok=True)
    pathlib.Path(args.json_output).write_text(
        json.dumps(
            {
                "summary": summary,
                "header_root": str(header_root),
                "results": results,
                "skipped_host": skipped_host,
                "setup_failures": failed_setup,
            },
            indent=2,
            sort_keys=True,
        )
        + "\n"
    )

    print(f"Wrote markdown report to {args.output}")
    print(f"Wrote json report to {args.json_output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

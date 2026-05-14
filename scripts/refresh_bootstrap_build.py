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


HIGH_RISK_PATHS = {
    "dev/src/callsem_output.h",
    "dev/src/callsemantic.cpp",
    "dev/src/lowirgensemantic.cpp",
    "dev/src/runtime_symbol_policy.cpp",
    "dev/src/semantic_context.h",
    "dev/src/semantic_expression.cpp",
    "dev/src/semantic_lifetime.cpp",
    "dev/src/semantic_model.h",
    "dev/src/symbol_linkage.cpp",
    "dev/src/template_resolution.cpp",
}
HIGH_FANOUT_LIMIT = 20


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--host-cxx",
                        default=os.environ.get("CPPGM_HOST_CXX",
                                               "/usr/local/opt/llvm/bin/clang++"))
    parser.add_argument("--frontend", default="dev/cppgm++.cpp")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--build-prefix", default="/tmp/bootstrap-selfhost-frontier-cluster")
    parser.add_argument("--output-prefix", default="")
    parser.add_argument("--jobs", type=int, default=max(os.cpu_count() or 1, 1))
    parser.add_argument("--changed", action="append", default=[],
                        help="Repo-relative changed path(s) used to infer the refresh closure")
    parser.add_argument("--source", action="append", default=[],
                        help="Repo-relative bootstrap source(s) to refresh directly")
    parser.add_argument("--skip-relink", action="store_true",
                        help="Refresh objects only; do not rerun the preserved link/runtime probe")
    return parser.parse_args()


def source_list(repo_root: Path, frontend: str):
    files = sorted((repo_root / "dev" / "src").glob("*.cpp"))
    files.append((repo_root / frontend).resolve())
    return files


def source_relpaths(repo_root: Path, frontend: str):
    return [str(src.relative_to(repo_root)) for src in source_list(repo_root, frontend)]


def object_path(build_dir: Path, index: int, src: Path):
    return build_dir / (f"{index:03d}-{src.stem}.o")


def normalize_repo_path(repo_root: Path, value: str) -> str:
    root = repo_root.resolve(strict=False)
    path = Path(value)
    if path.is_absolute():
        resolved = path.resolve(strict=False)
    else:
        resolved = (root / path).resolve(strict=False)
    try:
        return str(resolved.relative_to(root))
    except ValueError:
        return ""


def is_bootstrap_input(path: str) -> bool:
    return path.startswith("dev/") and Path(path).suffix in {".cpp", ".h", ".inc"}


def infer_changed_paths(repo_root: Path) -> list:
    cmd = [
        "git",
        "status",
        "--porcelain",
        "--untracked-files=normal",
        "--",
        "dev",
    ]
    proc = subprocess.run(
        cmd,
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        return []

    changed = []
    seen = set()
    for line in proc.stdout.splitlines():
        if not line:
            continue
        payload = line[3:]
        candidates = payload.split(" -> ") if " -> " in payload else [payload]
        for item in candidates:
            normalized = normalize_repo_path(repo_root, item)
            if not normalized or not is_bootstrap_input(normalized):
                continue
            if normalized in seen:
                continue
            seen.add(normalized)
            changed.append(normalized)
    return changed


def depfile_path(repo_root: Path, src: Path) -> Path:
    return repo_root / "obj" / "release" / ".d" / f"{src.stem}.d"


def parse_dependency_text(text: str, repo_root: Path, base_dir: Path) -> set:
    root = repo_root.resolve(strict=False)
    collapsed = re.sub(r"\\\s*\n", " ", text)
    if ":" not in collapsed:
        return set()
    _, deps_text = collapsed.split(":", 1)
    deps = set()
    for token in deps_text.split():
        if token == "\\":
            continue
        resolved = (base_dir / token).resolve(strict=False)
        try:
            deps.add(str(resolved.relative_to(root)))
        except ValueError:
            continue
    return deps


def host_dependency_scan(repo_root: Path, host_cxx: str, src: Path) -> set:
    cmd = [
        host_cxx,
        "-std=c++11",
        "-I",
        "dev/src",
        "-MM",
        str(src.relative_to(repo_root)),
    ]
    proc = subprocess.run(
        cmd,
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        raise RuntimeError(proc.stdout.strip() or "dependency scan failed")
    return parse_dependency_text(proc.stdout, repo_root, repo_root)


def load_dependency_map(repo_root: Path, host_cxx: str, sources) -> tuple:
    deps_by_source = {}
    notes = []
    depfile_base = repo_root / "dev"
    for src in sources:
        rel = str(src.relative_to(repo_root))
        dep_path = depfile_path(repo_root, src)
        deps = set()
        if dep_path.exists():
            deps = parse_dependency_text(dep_path.read_text(), repo_root, depfile_base)
        else:
            try:
                deps = host_dependency_scan(repo_root, host_cxx, src)
                notes.append(f"host-dep-scan:{rel}")
            except RuntimeError as exc:
                notes.append(f"dep-scan-failed:{rel}:{exc}")
        deps.add(rel)
        deps_by_source[rel] = deps
    reverse = {}
    for rel, deps in deps_by_source.items():
        for dep in deps:
            reverse.setdefault(dep, set()).add(rel)
    return deps_by_source, reverse, notes


def requested_sources(repo_root: Path, source_paths: list, explicit_sources: list) -> list:
    source_set = set(source_paths)
    requested = []
    seen = set()
    for item in explicit_sources:
        normalized = normalize_repo_path(repo_root, item)
        if normalized not in source_set:
            raise SystemExit("unknown bootstrap source: " + item)
        if normalized in seen:
            continue
        seen.add(normalized)
        requested.append(normalized)
    return requested


def changed_paths(repo_root: Path, explicit_changes: list) -> tuple:
    if explicit_changes:
        out = []
        seen = set()
        for item in explicit_changes:
            normalized = normalize_repo_path(repo_root, item)
            if not normalized:
                continue
            if normalized in seen:
                continue
            seen.add(normalized)
            out.append(normalized)
        return out, False
    return infer_changed_paths(repo_root), True


def affected_sources(repo_root: Path,
                     source_paths: list,
                     requested: list,
                     changed: list,
                     reverse_deps: dict) -> tuple:
    affected = set(requested)
    no_effect = []
    fanout = {}

    for path in changed:
        impacted = reverse_deps.get(path, set())
        fanout[path] = len(impacted)
        if path in source_paths:
            impacted = set(impacted)
            impacted.add(path)
        if impacted:
            affected.update(impacted)
        else:
            no_effect.append(path)

    ordered = [path for path in source_paths if path in affected]
    return ordered, no_effect, fanout


def recommended_followup(changed: list, fanout: dict) -> tuple:
    reasons = []
    hot = sorted(path for path in changed if path in HIGH_RISK_PATHS)
    if hot:
        reasons.append("hot-path:" + ",".join(hot))
    broad = sorted(path for path, count in fanout.items() if count >= HIGH_FANOUT_LIMIT)
    if broad:
        reasons.append("high-fanout:" + ",".join(broad))
    return ("full-frontier" if reasons else "refresh-relink"), reasons


def run(cmd, cwd: Path, env):
    started = time.time()
    proc = subprocess.run(
        cmd,
        cwd=str(cwd),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    return {
        "cmd": cmd,
        "returncode": proc.returncode,
        "stdout": proc.stdout,
        "duration_sec": time.time() - started,
    }


def compile_source(repo_root: Path, compiler: str, src: str, obj: Path, env):
    cmd = [
        str((repo_root / compiler).resolve()),
        "-c",
        "-I",
        str((repo_root / "dev" / "src").resolve()),
        "-o",
        str(obj),
        str((repo_root / src).resolve()),
    ]
    result = run(cmd, repo_root, env)
    result["source"] = src
    result["object"] = str(obj)
    return result


def refresh_objects(repo_root: Path,
                    compiler: str,
                    build_dir: Path,
                    source_paths: list,
                    sources_to_refresh: list,
                    jobs: int,
                    env) -> list:
    source_index = {source: index for index, source in enumerate(source_paths)}
    stages = []
    with ThreadPoolExecutor(max_workers=max(1, min(jobs, len(sources_to_refresh) or 1))) as executor:
        futures = {}
        for source in sources_to_refresh:
            index = source_index[source]
            src_path = (repo_root / source).resolve()
            obj = object_path(build_dir, index, src_path)
            future = executor.submit(compile_source, repo_root, compiler, source, obj, env)
            futures[future] = index
        for future in as_completed(futures):
            stage = future.result()
            stage["source_index"] = futures[future]
            stages.append(stage)
    stages.sort(key=lambda item: item["source_index"])
    return stages


def trim_output(text: str, limit: int = 40) -> str:
    lines = text.splitlines()
    if len(lines) <= limit:
        return text
    return "\n".join(lines[:limit] + ["[... truncated ...]"])


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)


def write_json(path: Path, payload: dict) -> None:
    write_text(path, json.dumps(payload, indent=2, sort_keys=True) + "\n")


def write_markdown(path: Path, report: dict) -> None:
    lines = []
    lines.append("# Bootstrap Refresh")
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- result: `{report['result']}`")
    lines.append(f"- build-prefix: `{report['build_prefix']}`")
    lines.append(f"- build-dir: `{report['build_dir']}`")
    lines.append(f"- changed-files: `{len(report['changed_files'])}`")
    lines.append(f"- affected-sources: `{len(report['affected_sources'])}`")
    lines.append(f"- refresh-jobs: `{report['jobs']}`")
    lines.append(f"- recommended-followup: `{report['recommended_followup']}`")
    if report["followup_reasons"]:
        lines.append(f"- followup-reasons: `{', '.join(report['followup_reasons'])}`")
    lines.append("")

    lines.append("## Changed Files")
    lines.append("")
    if not report["changed_files"]:
        lines.append("- none")
    else:
        for item in report["changed_files"]:
            suffix = ""
            if item in report["fanout"]:
                suffix = f" (fanout: {report['fanout'][item]})"
            lines.append(f"- `{item}`{suffix}")
    lines.append("")

    lines.append("## Affected Sources")
    lines.append("")
    if not report["affected_sources"]:
        lines.append("- none")
    else:
        for item in report["affected_sources"]:
            lines.append(f"- `{item}`")
    lines.append("")

    lines.append("## Compile Refresh")
    lines.append("")
    if not report["compile_results"]:
        lines.append("- none")
    else:
        for item in report["compile_results"]:
            lines.append(
                f"- `{item['source']}` ({item['duration_sec']:.1f}s, rc={item['returncode']})")
            if item["returncode"] != 0:
                lines.append("")
                lines.append("```")
                lines.append(trim_output(item.get("stdout", "")).rstrip())
                lines.append("```")
    lines.append("")

    if report.get("frontier_report"):
        frontier = report["frontier_report"]
        lines.append("## Relink Probe")
        lines.append("")
        lines.append(f"- result: `{frontier.get('result', '')}`")
        lines.append(f"- active-frontier: `{frontier.get('active_frontier', '')}`")
        if frontier.get("json_path"):
            lines.append(f"- json: `{frontier['json_path']}`")
        if frontier.get("md_path"):
            lines.append(f"- markdown: `{frontier['md_path']}`")
        lines.append("")

    if report["notes"]:
        lines.append("## Notes")
        lines.append("")
        for item in report["notes"]:
            lines.append(f"- `{item}`")
        lines.append("")

    write_text(path, "\n".join(lines))


def rerun_link_probe(repo_root: Path,
                     compiler: str,
                     host_cxx: str,
                     build_prefix: str,
                     env) -> dict:
    cmd = [
        sys.executable,
        str((repo_root / "scripts" / "report_bootstrap_frontier.py").resolve()),
        "--compiler",
        compiler,
        "--host-cxx",
        host_cxx,
        "--jobs",
        "1",
        "--output-prefix",
        build_prefix,
        "--skip-compile",
    ]
    stage = run(cmd, repo_root, env)
    report = {
        "command": cmd,
        "returncode": stage["returncode"],
        "stdout": stage["stdout"],
        "duration_sec": stage["duration_sec"],
        "json_path": build_prefix + ".json",
        "md_path": build_prefix + ".md",
    }
    json_path = Path(report["json_path"])
    if json_path.exists():
        try:
            payload = json.loads(json_path.read_text())
            report["result"] = payload.get("result", "")
            report["active_frontier"] = payload.get("active_frontier", "")
        except Exception:
            report["result"] = ""
            report["active_frontier"] = ""
    return report


def main():
    args = parse_args()
    if args.jobs < 1:
        raise SystemExit("--jobs must be >= 1")

    repo_root = Path(args.repo_root).resolve()
    build_prefix = args.build_prefix
    output_prefix = args.output_prefix or (build_prefix + ".refresh")
    build_dir = Path(build_prefix + ".build").resolve()
    if not build_dir.exists():
        raise SystemExit("missing preserved bootstrap build dir: " + str(build_dir))

    output_json = Path(output_prefix + ".json")
    output_md = Path(output_prefix + ".md")

    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = args.host_cxx

    sources = source_list(repo_root, args.frontend)
    source_paths = source_relpaths(repo_root, args.frontend)
    requested = requested_sources(repo_root, source_paths, args.source)
    changed, auto_changed = changed_paths(repo_root, args.changed)
    _deps_by_source, reverse_deps, dep_notes = load_dependency_map(repo_root, args.host_cxx, sources)
    impacted, no_effect, fanout = affected_sources(
        repo_root,
        source_paths,
        requested,
        changed,
        reverse_deps)
    followup, followup_reasons = recommended_followup(changed, fanout)

    report = {
        "compiler": args.compiler,
        "host_cxx": args.host_cxx,
        "build_prefix": build_prefix,
        "build_dir": str(build_dir),
        "output_prefix": output_prefix,
        "jobs": args.jobs,
        "auto_changed": auto_changed,
        "changed_files": changed,
        "requested_sources": requested,
        "affected_sources": impacted,
        "fanout": fanout,
        "recommended_followup": followup,
        "followup_reasons": followup_reasons,
        "compile_results": [],
        "notes": dep_notes + [f"no-bootstrap-impact:{item}" for item in no_effect],
        "result": "unknown",
    }

    if not changed and not requested:
        report["result"] = "no-changes"
        report["notes"].append("no bootstrap-relevant dev/ changes detected")
        write_json(output_json, report)
        write_markdown(output_md, report)
        return 0

    if not impacted:
        report["result"] = "no-bootstrap-impact"
        write_json(output_json, report)
        write_markdown(output_md, report)
        return 0

    compile_results = refresh_objects(
        repo_root,
        args.compiler,
        build_dir,
        source_paths,
        impacted,
        args.jobs,
        env,
    )
    report["compile_results"] = compile_results
    failed = next((item for item in compile_results if item["returncode"] != 0), None)
    if failed:
        report["result"] = "compile-failed"
        write_json(output_json, report)
        write_markdown(output_md, report)
        return 1

    if args.skip_relink:
        report["result"] = "refresh-complete"
        write_json(output_json, report)
        write_markdown(output_md, report)
        return 0

    frontier = rerun_link_probe(
        repo_root,
        args.compiler,
        args.host_cxx,
        build_prefix,
        env,
    )
    report["frontier_report"] = frontier
    report["result"] = frontier.get("result", "") or (
        "relink-failed" if frontier["returncode"] != 0 else "refresh-complete")
    write_json(output_json, report)
    write_markdown(output_md, report)
    return 0 if frontier["returncode"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())

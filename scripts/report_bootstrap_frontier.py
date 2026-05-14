#!/usr/bin/env python3

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import time

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from bootstrap_selfhost_layout import compile_command
from bootstrap_selfhost_layout import object_path as layout_object_path
from bootstrap_selfhost_layout import source_list as layout_source_list


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--host-cxx",
                        default=os.environ.get("CPPGM_HOST_CXX",
                                               "/usr/local/opt/llvm/bin/clang++"))
    parser.add_argument("--jobs", type=int, default=1)
    parser.add_argument("--frontend", default="dev/cppgm++.cpp")
    parser.add_argument("--output-prefix", default="/tmp/bootstrap-selfhost-frontier")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--resume-state", default="")
    parser.add_argument("--reset-resume-state", action="store_true")
    parser.add_argument("--skip-compile", action="store_true")
    parser.add_argument("--build-dir", default="")
    parser.add_argument("--object-layout", default="scratch",
                        choices=["scratch", "pa37-selfhost"])
    parser.add_argument("--test-runner", type=int,
                        default=int(os.environ.get("CPPGM_TEST_RUNNER", "1") or "1"))
    parser.add_argument("--stdlib-flags",
                        default=os.environ.get("CPPGM_STDLIB_FLAGS", ""))
    parser.add_argument("--lookahead-failures", type=int, default=3)
    parser.add_argument("--cleanup-build-dir", action="store_true")
    parser.add_argument("--debugger", default=os.environ.get("CPPGM_DEBUGGER", ""))
    return parser.parse_args()


def source_list(repo_root: Path, frontend: str):
    return layout_source_list(repo_root, frontend, "scratch", False)


def source_relpaths(repo_root: Path, frontend: str):
    return [str(src.relative_to(repo_root)) for src in source_list(repo_root, frontend)]


def tracked_input_files(repo_root: Path, frontend: str, object_layout: str, test_runner: bool):
    files = set(layout_source_list(repo_root, frontend, object_layout, test_runner))
    for pattern in ("dev/*.cpp", "dev/*.h", "dev/*.inc", "dev/src/*.h", "dev/src/*.inc"):
        files.update(path for path in repo_root.glob(pattern) if path.is_file())
    return sorted(files)


def input_manifest(repo_root: Path,
                   compiler_path: Path,
                   frontend: str,
                   object_layout: str,
                   test_runner: bool):
    manifest = {
        "compiler": {},
        "inputs": [],
    }
    if compiler_path.exists():
        stat = compiler_path.stat()
        manifest["compiler"] = {
            "path": str(compiler_path.relative_to(repo_root)),
            "mtime_ns": stat.st_mtime_ns,
            "size": stat.st_size,
        }
    for path in tracked_input_files(repo_root, frontend, object_layout, test_runner):
        stat = path.stat()
        manifest["inputs"].append({
            "path": str(path.relative_to(repo_root)),
            "mtime_ns": stat.st_mtime_ns,
            "size": stat.st_size,
        })
    return manifest

def run(cmd, cwd: Path, env):
    started = time.time()
    proc = subprocess.run(
        cmd,
        cwd=str(cwd),
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return {
        "cmd": cmd,
        "returncode": proc.returncode,
        "stdout": proc.stdout,
        "duration_sec": time.time() - started,
    }


def trim_output(text: str, limit: int = 120):
    lines = text.splitlines()
    if len(lines) <= limit:
      return text
    return "\n".join(lines[:limit] + ["[... truncated ...]"])


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(65536)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def rotate_output_family(current_prefix: str,
                         previous_prefix: str,
                         suffixes) -> dict:
    moved = {}
    for suffix in suffixes:
        current_path = Path(current_prefix + suffix)
        if not current_path.exists():
            continue
        previous_path = Path(previous_prefix + suffix)
        previous_path.parent.mkdir(parents=True, exist_ok=True)
        if previous_path.exists():
            previous_path.unlink()
        current_path.replace(previous_path)
        moved[suffix] = str(previous_path)
    return moved


def load_cached_trace_analysis(analysis_prefix: str,
                               cluster_json: Path,
                               trace_tool: Path) -> dict:
    cache_json = Path(analysis_prefix + ".json")
    text_path = Path(analysis_prefix + ".txt")
    detail_path = Path(analysis_prefix + ".details.txt")
    if not cache_json.exists() or not text_path.exists() or not detail_path.exists():
        return {}
    try:
        payload = json.loads(cache_json.read_text())
    except Exception:
        return {}

    meta = payload.get("meta", {})
    request = meta.get("request", {})
    inputs = meta.get("inputs", {})
    tool = meta.get("tool", {})

    if request.get("cluster") != str(cluster_json.resolve()):
        return {}
    if inputs.get("cluster_sha256") != file_sha256(cluster_json):
        return {}
    if tool.get("path") != str(trace_tool.resolve()):
        return {}
    if tool.get("sha256") != file_sha256(trace_tool):
        return {}

    return {
        "text": str(text_path),
        "detail": str(detail_path),
        "json": str(cache_json),
    }


def load_trace_frontier_snapshot(trace_json_path: str) -> dict:
    path = Path(trace_json_path)
    if not path.exists():
        return {}
    try:
        payload = json.loads(path.read_text())
    except Exception:
        return {}

    summary_data = {}
    integration_data = {}
    for section in payload.get("sections", []):
        if not isinstance(section, dict):
            continue
        kind = section.get("kind")
        data = section.get("data")
        if not isinstance(data, dict):
            continue
        if kind == "frontier_summary" and not summary_data:
            summary_data = data
        elif kind == "frontier_integration" and not integration_data:
            integration_data = data

    if not summary_data and not integration_data:
        return {}

    snapshot = {
        "issue_kind": integration_data.get("issue_kind",
                                            summary_data.get("primary_issue_kind", "")),
        "undefined_symbols": summary_data.get("undefined_symbol_count", 0),
        "duplicate_symbols": summary_data.get("duplicate_symbol_count", 0),
        "family_counts": summary_data.get("family_counts", {}),
        "symbols": summary_data.get("symbols", []),
        "candidate_count": integration_data.get("candidate_count", 0),
        "candidates": integration_data.get("candidates", []),
    }
    if integration_data.get("best_candidate"):
        snapshot["best_candidate"] = integration_data["best_candidate"]
    return snapshot


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
        return line[:200]
    return "<no output>"


def signal_name_for_returncode(returncode: int) -> str:
    if returncode >= 0:
        return ""
    signum = -returncode
    try:
        return signal.Signals(signum).name
    except Exception:
        return f"SIG{signum}"


def resolve_debugger(explicit: str) -> dict:
    candidates = []
    if explicit:
        candidates.append(explicit)
    candidates.extend(["lldb", "gdb"])
    for candidate in candidates:
        resolved = shutil.which(candidate)
        if not resolved:
            continue
        name = Path(resolved).name
        kind = "gdb" if name.startswith("gdb") else "lldb"
        return {"path": resolved, "kind": kind}
    return {}


def debugger_command(debugger: dict, cmd) -> list:
    debugger_path = debugger["path"]
    if debugger["kind"] == "gdb":
        return [
            debugger_path,
            "--batch",
            "-ex",
            "set pagination off",
            "-ex",
            "run",
            "-ex",
            "thread apply all bt full",
            "--args",
        ] + list(cmd)
    return [
        debugger_path,
        "--batch",
        "-o",
        "settings set target.process.stop-on-exec false",
        "-o",
        "run",
        "-o",
        "thread backtrace all",
        "--",
    ] + list(cmd)


def extract_backtrace_frames(text: str, limit: int = 8) -> list:
    frames = []
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped:
            continue
        if re.match(r"^(?:\*\s+)?frame #\d+:", stripped):
            frames.append(stripped)
        elif re.match(r"^#\d+\s", stripped):
            frames.append(stripped)
        if len(frames) >= limit:
            break
    return frames


def write_crash_capture(path: Path,
                        debugger: dict,
                        cmd,
                        cwd: Path,
                        returncode: int,
                        output: str) -> None:
    lines = [
        f"debugger: {debugger['path']}",
        f"cwd: {cwd}",
        f"command: {' '.join(cmd)}",
        f"returncode: {returncode}",
        "",
        output.rstrip(),
        "",
    ]
    path.write_text("\n".join(lines))


def capture_debugger_backtrace(debugger: dict,
                               cmd,
                               cwd: Path,
                               env,
                               output_path: Path) -> dict:
    debug_cmd = debugger_command(debugger, cmd)
    try:
        proc = subprocess.run(
            debug_cmd,
            cwd=str(cwd),
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=90,
        )
        output = proc.stdout
        timed_out = False
    except subprocess.TimeoutExpired as exc:
        output = (exc.stdout or "") + "\n[debugger timed out]\n"
        proc = None
        timed_out = True

    output_path.parent.mkdir(parents=True, exist_ok=True)
    write_crash_capture(
        output_path,
        debugger,
        debug_cmd,
        cwd,
        proc.returncode if proc is not None else -1,
        output)
    return {
        "path": str(output_path),
        "debugger": debugger["path"],
        "returncode": proc.returncode if proc is not None else -1,
        "timed_out": timed_out,
        "top_frames": extract_backtrace_frames(output),
    }


def active_failure_stage(report: dict) -> dict:
    failures = [stage for stage in report.get("stages", []) if stage.get("returncode", 0) != 0]
    if not failures:
        return {}

    active_frontier = report.get("active_frontier", "")
    for stage in failures:
        if active_frontier == stage.get("source"):
            return stage
        if active_frontier == stage.get("stage"):
            return stage
        if active_frontier == "host-link" and stage.get("stage") == "link":
            return stage
    return failures[0]


def write_stage_output_capture(path: Path, stage: dict) -> None:
    lines = [
        f"stage: {stage.get('stage', '')}",
        f"source: {stage.get('source', '')}",
        f"returncode: {stage.get('returncode', '')}",
        f"duration_sec: {stage.get('duration_sec', '')}",
        f"command: {' '.join(stage.get('cmd', []))}",
        "",
        (stage.get("stdout", "") or "").rstrip(),
        "",
    ]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines))


def collect_failure_output_artifacts(report: dict, output_prefix: str) -> dict:
    stage = active_failure_stage(report)
    if not stage:
        return {}

    stage_name = stage.get("stage", "stage")
    output_path = Path(f"{output_prefix}.{stage_name}.stdout.txt")
    write_stage_output_capture(output_path, stage)
    return {
        "stage": stage_name,
        "source": stage.get("source", ""),
        "path": str(output_path),
    }


def collect_crash_artifacts(report: dict,
                            output_prefix: str,
                            repo_root: Path,
                            env,
                            debugger: dict) -> dict:
    if not report.get("stages"):
        return {}
    stage = report["stages"][-1]
    if stage.get("returncode", 0) >= 0:
        return {}

    signal_name = signal_name_for_returncode(stage["returncode"])
    crash_info = {
        "stage": stage.get("stage", ""),
        "signal": -stage["returncode"],
        "signal_name": signal_name,
        "command": stage.get("cmd", []),
    }

    if debugger:
        stage_name = stage.get("stage", "stage")
        exact_path = Path(f"{output_prefix}.{stage_name}.crash.txt")
        crash_info["exact_command_backtrace"] = capture_debugger_backtrace(
            debugger,
            stage["cmd"],
            repo_root,
            env,
            exact_path)
    else:
        crash_info["debugger_unavailable"] = True

    if stage.get("stage") == "self-compile-smoke" and stage.get("cmd"):
        self_binary_cmd = [stage["cmd"][0]]
        probe = run(self_binary_cmd, repo_root, env)
        crash_info["self_binary_noargs_probe"] = {
            "command": self_binary_cmd,
            "returncode": probe["returncode"],
            "duration_sec": probe["duration_sec"],
            "stdout": trim_output(probe["stdout"]).rstrip(),
        }
        if probe["returncode"] < 0:
            crash_info["self_binary_noargs_probe"]["signal"] = -probe["returncode"]
            crash_info["self_binary_noargs_probe"]["signal_name"] = (
                signal_name_for_returncode(probe["returncode"]))
            if debugger:
                noargs_path = Path(f"{output_prefix}.self-binary-noargs.crash.txt")
                crash_info["self_binary_noargs_backtrace"] = capture_debugger_backtrace(
                    debugger,
                    self_binary_cmd,
                    repo_root,
                    env,
                    noargs_path)
    return crash_info


def annotate_failure_summary(report, lookahead_failures: int):
    failures = [stage for stage in report["stages"] if stage["returncode"] != 0]
    if len(failures) <= 1:
        return

    first_failure = failures[0]
    next_failures = []
    for stage in failures[1:1 + max(lookahead_failures, 0)]:
        next_failures.append({
            "stage": stage["stage"],
            "source": stage.get("source", ""),
            "returncode": stage["returncode"],
            "signature": normalize_failure_output(stage.get("stdout", "")),
        })
    if next_failures:
        report["next_failures"] = next_failures

    clusters = {}
    for stage in failures:
        signature = normalize_failure_output(stage.get("stdout", ""))
        cluster = clusters.setdefault(signature, {
            "count": 0,
            "examples": [],
        })
        cluster["count"] += 1
        source = stage.get("source", stage["stage"])
        if len(cluster["examples"]) < 5:
            cluster["examples"].append(source)

    report["failure_clusters"] = [
        {
            "signature": signature,
            "count": cluster["count"],
            "examples": cluster["examples"],
        }
        for signature, cluster in sorted(
            clusters.items(),
            key=lambda item: (-item[1]["count"], item[0]),
        )
    ]


def load_resume_state(path: Path, args, source_paths, expected_input_manifest):
    if args.reset_resume_state or not path.exists():
        return None, ("requested-reset" if args.reset_resume_state else "missing-state")
    try:
        state = json.loads(path.read_text())
    except Exception:
        return None, "unreadable-state"
    expected = {
        "compiler": args.compiler,
        "host_cxx": args.host_cxx,
        "repo_root": str(Path(args.repo_root).resolve()),
        "frontend": args.frontend,
        "object_layout": args.object_layout,
        "sources": source_paths,
        "stdlib_flags": args.stdlib_flags,
        "test_runner": bool(args.test_runner),
    }
    for key, value in expected.items():
        if state.get(key) != value:
            return None, f"state-mismatch:{key}"
    if state.get("input_manifest") != expected_input_manifest:
        return None, "inputs-changed"
    next_index = state.get("next_index", 0)
    if not isinstance(next_index, int) or next_index < 0 or next_index > len(source_paths):
        return None, "invalid-next-index"
    build_dir = state.get("build_dir", "")
    if not build_dir:
        return None, "missing-build-dir"
    return state, None


def save_resume_state(path: Path,
                      args,
                      source_paths,
                      expected_input_manifest,
                      build_dir: Path,
                      next_index: int):
    path.parent.mkdir(parents=True, exist_ok=True)
    state = {
        "build_dir": str(build_dir),
        "compiler": args.compiler,
        "frontend": args.frontend,
        "host_cxx": args.host_cxx,
        "input_manifest": expected_input_manifest,
        "next_index": next_index,
        "object_layout": args.object_layout,
        "repo_root": str(Path(args.repo_root).resolve()),
        "sources": source_paths,
        "stdlib_flags": args.stdlib_flags,
        "test_runner": bool(args.test_runner),
    }
    path.write_text(json.dumps(state, indent=2, sort_keys=True) + "\n")


def compile_source(repo_root: Path, compiler: str, include_flag: str, src: Path, obj: Path, env):
    del include_flag
    return run(compile_command(repo_root,
                               compiler,
                               src,
                               obj,
                               "scratch",
                               False,
                               ""),
               repo_root,
               env)


def compile_sources_parallel(repo_root: Path, compiler: str, include_flag: str,
                             sources, source_paths, build_dir: Path, env, jobs: int):
    stages = []
    with ThreadPoolExecutor(max_workers=jobs) as executor:
        futures = {}
        for index, src in enumerate(sources):
            obj = layout_object_path(repo_root, build_dir, index, src, "scratch", False)
            future = executor.submit(compile_source, repo_root, compiler, include_flag,
                                     src, obj, env)
            futures[future] = (index, src, obj)

        for future in as_completed(futures):
            index, src, obj = futures[future]
            stage = future.result()
            stage["stage"] = "compile"
            stage["source"] = source_paths[index]
            stage["object"] = str(obj)
            stage["source_index"] = index
            stages.append(stage)

    stages.sort(key=lambda stage: stage["source_index"])
    return stages


def main():
    args = parse_args()
    if args.jobs < 1:
        raise SystemExit("--jobs must be >= 1")
    repo_root = Path(args.repo_root).resolve()
    env = os.environ.copy()
    env["CPPGM_HOST_CXX"] = args.host_cxx
    sources = layout_source_list(repo_root,
                                 args.frontend,
                                 args.object_layout,
                                 bool(args.test_runner))
    source_paths = [str(src.relative_to(repo_root)) for src in sources]
    compiler_path = (repo_root / args.compiler).resolve()
    expected_input_manifest = input_manifest(repo_root,
                                             compiler_path,
                                             args.frontend,
                                             args.object_layout,
                                             bool(args.test_runner))
    resume_state_path = Path(args.resume_state).resolve() if args.resume_state else None
    resume_reset_reason = None
    resume_state = None
    if resume_state_path:
        resume_state, resume_reset_reason = load_resume_state(
            resume_state_path, args, source_paths, expected_input_manifest)
    if resume_state_path:
        if resume_state:
            tmpdir = Path(resume_state["build_dir"])
        elif args.build_dir:
            tmpdir = Path(args.build_dir).resolve()
        else:
            tmpdir = Path(args.output_prefix + ".build").resolve()
            if not args.skip_compile:
                shutil.rmtree(tmpdir, ignore_errors=True)
        tmpdir.mkdir(parents=True, exist_ok=True)
    else:
        tmpdir = (Path(args.build_dir).resolve()
                  if args.build_dir
                  else Path(args.output_prefix + ".build").resolve())
        if not args.skip_compile:
            shutil.rmtree(tmpdir, ignore_errors=True)
        tmpdir.mkdir(parents=True, exist_ok=True)
    report = {
        "compiler": args.compiler,
        "frontend": args.frontend,
        "host_cxx": args.host_cxx,
        "jobs": args.jobs,
        "repo_root": str(repo_root),
        "build_dir": str(tmpdir),
        "mode": ("link-only" if args.skip_compile
                  else ("resume-fast" if resume_state_path
                        else ("parallel" if args.jobs > 1 else "serial"))),
        "stages": [],
        "result": "unknown",
    }
    archived_previous_reports = rotate_output_family(
        args.output_prefix,
        args.output_prefix + ".prev",
        [".md", ".json"],
    )
    if archived_previous_reports:
        report["archived_previous_reports"] = archived_previous_reports
    archived_legacy_sidecars = rotate_output_family(
        args.output_prefix,
        args.output_prefix + ".prev",
        [".txt", ".details.txt"],
    )
    if archived_legacy_sidecars:
        report["archived_legacy_sidecars"] = archived_legacy_sidecars
    archived_crash_sidecars = rotate_output_family(
        args.output_prefix,
        args.output_prefix + ".prev",
        [
            ".compile.stdout.txt",
            ".compile-backfill.stdout.txt",
            ".link.stdout.txt",
            ".self-compile-smoke.stdout.txt",
            ".self-run-smoke.stdout.txt",
            ".compile.crash.txt",
            ".link.crash.txt",
            ".self-compile-smoke.crash.txt",
            ".self-run-smoke.crash.txt",
            ".self-binary-noargs.crash.txt",
        ],
    )
    if archived_crash_sidecars:
        report["archived_crash_sidecars"] = archived_crash_sidecars
    start_index = resume_state.get("next_index", 0) if resume_state else 0
    if resume_state_path:
        report["trusted_prefix_count"] = start_index
        report["resume_state"] = str(resume_state_path)
        if resume_reset_reason and not resume_state:
            report["resume_state_reset_reason"] = resume_reset_reason

    try:
        include_flag = str(repo_root / "dev" / "src")
        if args.skip_compile:
            missing_objects = []
            for index, src in enumerate(sources):
                obj = layout_object_path(repo_root,
                                         tmpdir,
                                         index,
                                         src,
                                         args.object_layout,
                                         bool(args.test_runner))
                if not obj.exists():
                    missing_objects.append(str(src.relative_to(repo_root)))
            if missing_objects:
                report["result"] = "compile-artifacts-missing"
                report["active_frontier"] = missing_objects[0]
                report["missing_objects"] = missing_objects
        elif not resume_state_path and args.jobs > 1:
            stages = compile_sources_parallel(repo_root, args.compiler, include_flag,
                                              sources, source_paths, tmpdir, env, args.jobs)
            report["stages"].extend(stages)
            for stage in stages:
                if stage["returncode"] != 0:
                    report["result"] = "compile-failed"
                    report["active_frontier"] = stage["source"]
                    break
        else:
            for index in range(start_index, len(sources)):
                src = sources[index]
                obj = layout_object_path(repo_root, tmpdir, index, src, "scratch", False)
                stage = compile_source(repo_root, args.compiler, include_flag, src, obj, env)
                stage["stage"] = "compile"
                stage["source"] = source_paths[index]
                stage["object"] = str(obj)
                stage["source_index"] = index
                report["stages"].append(stage)
                if stage["returncode"] != 0:
                    report["result"] = "compile-failed"
                    report["active_frontier"] = stage["source"]
                    if resume_state_path:
                        save_resume_state(
                            resume_state_path,
                            args,
                            source_paths,
                            expected_input_manifest,
                            tmpdir,
                            index)
                    break
                if resume_state_path:
                    save_resume_state(
                        resume_state_path,
                        args,
                        source_paths,
                        expected_input_manifest,
                        tmpdir,
                        index + 1)

        if report["result"] == "unknown" and resume_state_path:
            for index in range(start_index):
                src = sources[index]
                obj = layout_object_path(repo_root, tmpdir, index, src, "scratch", False)
                if obj.exists():
                    continue
                stage = compile_source(repo_root, args.compiler, include_flag, src, obj, env)
                stage["stage"] = "compile-backfill"
                stage["source"] = source_paths[index]
                stage["object"] = str(obj)
                stage["source_index"] = index
                report["stages"].append(stage)
                if stage["returncode"] != 0:
                    report["result"] = "compile-failed"
                    report["active_frontier"] = stage["source"]
                    save_resume_state(
                        resume_state_path,
                        args,
                        source_paths,
                        expected_input_manifest,
                        tmpdir,
                        index)
                    break

        if report["result"] == "unknown":
            objects = [str(layout_object_path(repo_root,
                                              tmpdir,
                                              index,
                                              src,
                                              args.object_layout,
                                              bool(args.test_runner)))
                       for index, src in enumerate(sources)]
            self_binary = tmpdir / "cppgm++-self"
            stage = run([args.host_cxx, "-o", str(self_binary)] + objects, repo_root, env)
            stage["stage"] = "link"
            stage["output"] = str(self_binary)
            report["stages"].append(stage)
            if stage["returncode"] != 0:
                report["result"] = "link-failed"
                report["active_frontier"] = "host-link"
            else:
                smoke_src = tmpdir / "bootstrap-ret0.cpp"
                smoke_src.write_text("int main() { return 0; }\n")
                smoke_bin = tmpdir / "bootstrap-ret0"
                stage = run([str(self_binary), "-o", str(smoke_bin), str(smoke_src)],
                            repo_root, env)
                stage["stage"] = "self-compile-smoke"
                stage["source"] = str(smoke_src)
                stage["output"] = str(smoke_bin)
                report["stages"].append(stage)
                if stage["returncode"] != 0:
                    report["result"] = "self-compile-smoke-failed"
                    report["active_frontier"] = "self-compile-smoke"
                else:
                    stage = run([str(smoke_bin)], repo_root, env)
                    stage["stage"] = "self-run-smoke"
                    report["stages"].append(stage)
                    report["result"] = ("success" if stage["returncode"] == 0
                                        else "self-run-smoke-failed")
                    report["active_frontier"] = ("none" if stage["returncode"] == 0
                                                  else "self-run-smoke")
            if resume_state_path and report["result"] in {
                "success",
                "link-failed",
                "self-compile-smoke-failed",
                "self-run-smoke-failed",
            }:
                save_resume_state(
                    resume_state_path,
                    args,
                    source_paths,
                    expected_input_manifest,
                    tmpdir,
                    len(sources))

        annotate_failure_summary(report, args.lookahead_failures)
        failure_output = collect_failure_output_artifacts(report, args.output_prefix)
        if failure_output:
            report["failure_output"] = failure_output
        debugger = resolve_debugger(args.debugger)
        crash_info = collect_crash_artifacts(report,
                                             args.output_prefix,
                                             repo_root,
                                             env,
                                             debugger)
        if crash_info:
            report["crash_analysis"] = crash_info

        md_path = Path(args.output_prefix + ".md")
        json_path = Path(args.output_prefix + ".json")
        md_path.parent.mkdir(parents=True, exist_ok=True)
        summary = [
            "# Bootstrap Self-Host Frontier Report",
            "",
            f"- Compiler: `{args.compiler}`",
            f"- Host compiler: `{args.host_cxx}`",
            f"- Jobs: `{args.jobs}`",
            f"- Repo root: `{repo_root}`",
            f"- Build dir: `{tmpdir}`",
            f"- Mode: `{report['mode']}`",
            f"- Result: `{report['result']}`",
            f"- Active frontier: `{report.get('active_frontier', '<unknown>')}`",
            "",
            "## Stages",
            "",
        ]
        if "trusted_prefix_count" in report:
            summary.insert(7, f"- Trusted prefix count: `{report['trusted_prefix_count']}`")
        if "resume_state" in report:
            summary.insert(8, f"- Resume state: `{report['resume_state']}`")
        if "resume_state_reset_reason" in report:
            summary.insert(9, f"- Resume state reset: `{report['resume_state_reset_reason']}`")
        for stage in report["stages"]:
            summary.append(f"### `{stage['stage']}`")
            if "source" in stage:
                summary.append(f"- Source: `{stage['source']}`")
            summary.append(f"- Return code: `{stage['returncode']}`")
            summary.append(f"- Duration: `{stage['duration_sec']:.2f}s`")
            summary.append(f"- Command: `{' '.join(stage['cmd'])}`")
            if stage.get("stdout"):
                summary.append("")
                summary.append("```text")
                summary.append(trim_output(stage["stdout"]).rstrip())
                summary.append("```")
            summary.append("")
        if report.get("next_failures"):
            summary.extend([
                "## Next Likely Failures",
                "",
            ])
            for stage in report["next_failures"]:
                summary.append(
                    f"- `{stage['source'] or stage['stage']}`: "
                    f"`{stage['signature']}`"
                )
            summary.append("")
        if report.get("failure_clusters"):
            summary.extend([
                "## Failure Clusters",
                "",
            ])
            for cluster in report["failure_clusters"]:
                examples = ", ".join(f"`{item}`" for item in cluster["examples"])
                summary.append(
                    f"- `{cluster['signature']}`: {cluster['count']} "
                    f"failure(s); examples: {examples}"
                )
            summary.append("")
        if report.get("failure_output"):
            output = report["failure_output"]
            summary.extend([
                "## Failure Output",
                "",
                f"- Stage: `{output['stage']}`",
                f"- Path: `{output['path']}`",
                "",
            ])
        if report.get("missing_objects"):
            summary.extend([
                "## Missing Compile Artifacts",
                "",
            ])
            for source in report["missing_objects"]:
                summary.append(f"- `{source}`")
            summary.append("")
        if report.get("crash_analysis"):
            crash = report["crash_analysis"]
            summary.extend([
                "## Crash Analysis",
                "",
                f"- Stage: `{crash['stage']}`",
                f"- Signal: `{crash['signal_name']} ({crash['signal']})`",
            ])
            if crash.get("exact_command_backtrace"):
                exact = crash["exact_command_backtrace"]
                summary.append(f"- Exact command backtrace: `{exact['path']}`")
                for frame in exact.get("top_frames", [])[:5]:
                    summary.append(f"- Top frame: `{frame}`")
            elif crash.get("debugger_unavailable"):
                summary.append("- Debugger: unavailable")
            probe = crash.get("self_binary_noargs_probe")
            if probe:
                probe_line = f"- Self-binary no-args probe: returncode `{probe['returncode']}`"
                if probe.get("signal_name"):
                    probe_line += f" (`{probe['signal_name']} ({probe['signal']})`)"
                summary.append(probe_line)
                if crash.get("self_binary_noargs_backtrace"):
                    summary.append(
                        f"- Self-binary no-args backtrace: `{crash['self_binary_noargs_backtrace']['path']}`"
                    )
            summary.append("")
        md_path.write_text("\n".join(summary) + "\n")
        json_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")

        if report["result"] == "link-failed":
            analysis_prefix = args.output_prefix + ".trace"
            trace_tool = (repo_root / "scripts" / "bootstrap_trace_report.py").resolve()
            analysis_cmd = [
                sys.executable,
                str(trace_tool),
                "--repo-root",
                str(repo_root),
                "--cluster",
                str(json_path),
                "--write-prefix",
                analysis_prefix,
            ]
            analysis_paths = {
                "text": analysis_prefix + ".txt",
                "detail": analysis_prefix + ".details.txt",
                "json": analysis_prefix + ".json",
            }
            cached_paths = load_cached_trace_analysis(analysis_prefix, json_path, trace_tool)
            if cached_paths:
                report["trace_analysis"] = {
                    "status": "ok",
                    "cmd": analysis_cmd,
                    "paths": cached_paths,
                    "reused": True,
                }
                summary.extend([
                    "## Trace Analysis",
                    "",
                    "- Reused existing persisted trace analysis",
                    "",
                    f"- Text: `{cached_paths['text']}`",
                    f"- Details: `{cached_paths['detail']}`",
                    f"- JSON: `{cached_paths['json']}`",
                    "",
                ])
            else:
                archived_trace_paths = rotate_output_family(
                    analysis_prefix,
                    analysis_prefix + ".prev",
                    [".txt", ".details.txt", ".json"],
                )
                analysis_stage = run(analysis_cmd, repo_root, env)
                if analysis_stage["returncode"] == 0:
                    report["trace_analysis"] = {
                        "status": "ok",
                        "cmd": analysis_cmd,
                        "paths": analysis_paths,
                        "reused": False,
                    }
                    if archived_trace_paths:
                        report["trace_analysis"]["archived_previous"] = archived_trace_paths
                    summary.extend([
                        "## Trace Analysis",
                        "",
                    ])
                    if archived_trace_paths:
                        summary.extend([
                            "- Archived previous trace analysis",
                            "",
                        ])
                    summary.extend([
                        f"- Text: `{analysis_paths['text']}`",
                        f"- Details: `{analysis_paths['detail']}`",
                        f"- JSON: `{analysis_paths['json']}`",
                        "",
                    ])
                else:
                    report["trace_analysis"] = {
                        "status": "failed",
                        "cmd": analysis_cmd,
                        "stdout": trim_output(analysis_stage["stdout"]).rstrip(),
                    }
                    if archived_trace_paths:
                        report["trace_analysis"]["archived_previous"] = archived_trace_paths
                    summary.extend([
                        "## Trace Analysis",
                        "",
                        "- Automatic trace analysis failed",
                        "",
                        "```text",
                        trim_output(analysis_stage["stdout"]).rstrip(),
                        "```",
                        "",
                    ])
            if report.get("trace_analysis", {}).get("status") == "ok":
                snapshot = load_trace_frontier_snapshot(
                    report["trace_analysis"]["paths"]["json"])
                if snapshot:
                    report["undefined_symbols"] = snapshot.get("undefined_symbols", 0)
                    report["duplicate_symbols"] = snapshot.get("duplicate_symbols", 0)
                    report["link_frontier"] = snapshot
                    summary.extend([
                        f"- Issue kind: `{snapshot.get('issue_kind', '<unknown>')}`",
                        f"- Undefined symbols: `{snapshot.get('undefined_symbols', 0)}`",
                        f"- Duplicate symbols: `{snapshot.get('duplicate_symbols', 0)}`",
                    ])
                    best_candidate = snapshot.get("best_candidate")
                    if best_candidate:
                        summary.append(
                            f"- Best candidate: `{best_candidate.get('demangled', best_candidate.get('raw', '<unknown>'))}`"
                        )
                    summary.append("")
            md_path.write_text("\n".join(summary) + "\n")
            json_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")

        print(f"Wrote markdown report to {md_path}")
        print(f"Wrote json report to {json_path}")
        if report.get("trace_analysis", {}).get("status") == "ok":
            print("Wrote trace analysis to "
                  f"{report['trace_analysis']['paths']['text']} and "
                  f"{report['trace_analysis']['paths']['json']}")
        if report.get("crash_analysis", {}).get("exact_command_backtrace", {}).get("path"):
            print("Wrote crash backtrace to "
                  f"{report['crash_analysis']['exact_command_backtrace']['path']}")
        if report.get("crash_analysis", {}).get("self_binary_noargs_backtrace", {}).get("path"):
            print("Wrote self-binary no-args backtrace to "
                  f"{report['crash_analysis']['self_binary_noargs_backtrace']['path']}")
        if report.get("failure_output", {}).get("path"):
            print("Wrote full failure output to "
                  f"{report['failure_output']['path']}")
        if report["result"] != "success":
            return 1
        return 0
    finally:
        if args.cleanup_build_dir:
            shutil.rmtree(tmpdir, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())

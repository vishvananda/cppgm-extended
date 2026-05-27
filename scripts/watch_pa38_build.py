#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import re
import shlex
import subprocess
import sys
import time
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Dict
from typing import Iterable
from typing import List
from typing import Optional
from typing import Sequence
from typing import Set


REPO_ROOT = Path(__file__).resolve().parent.parent
PA38_DIR = REPO_ROOT / "pa38"
FRONTEND_SOURCE_SETS = REPO_ROOT / "dev" / "frontend_source_sets.mk"
PA38_MAKEFILE = PA38_DIR / "Makefile"
MAKE_OPTIONS_WITH_ARGS = {
    "-C",
    "-f",
    "-I",
    "-j",
    "-l",
    "-o",
    "-W",
    "--directory",
    "--file",
    "--include-dir",
    "--jobs",
    "--load-average",
    "--makefile",
    "--max-load",
    "--new-file",
    "--old-file",
    "--assume-new",
    "--assume-old",
    "--what-if",
}


@dataclass
class ProcessInfo:
    pid: int
    ppid: int
    elapsed_seconds: int
    command: str
    argv: List[str]


@dataclass
class ActiveTask:
    phase: str
    label: str
    detail: str
    elapsed_seconds: Optional[int]
    output_path: Optional[Path] = None


@dataclass
class CheckpointStatus:
    name: str
    shared_done: int
    shared_total: int
    entry_done: bool
    binary_done: bool


@dataclass
class BuildSpec:
    target: str
    obj_root_base: Path
    bin_root_base: Path
    generated_root: Path
    flavor: str
    output_suffix: str
    test_runner: bool
    cxx_dep: Optional[Path]
    checkpoints: List[str]
    scope_label: str


@dataclass
class BuildView:
    build_id: str
    spec: BuildSpec
    object_root: Path
    bin_root: Path
    shared_total: int
    shared_done: int
    entry_total: int
    entry_done: int
    runner_total: int
    runner_done: int
    binary_total: int
    binary_done: int
    checkpoint_statuses: List[CheckpointStatus]
    active_tasks: List[ActiveTask]
    root_pid: Optional[int]
    root_elapsed_seconds: Optional[int]
    root_command: str


class Color:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    DIM = "\033[2m"
    RED = "\033[31m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    MAGENTA = "\033[35m"
    CYAN = "\033[36m"


def color_enabled(mode: str) -> bool:
    if mode == "always":
        return True
    if mode == "never":
        return False
    return sys.stdout.isatty() and not os.environ.get("NO_COLOR")


def style(text: str, *codes: str, enabled: bool) -> str:
    if not enabled or not codes:
        return text
    return "".join(codes) + text + Color.RESET


def run_capture(cmd: Sequence[str]) -> str:
    return subprocess.check_output(cmd, cwd=str(REPO_ROOT), text=True)


def parse_elapsed_text(text: str) -> int:
    text = text.strip()
    if not text:
        return 0
    days = 0
    if "-" in text:
        day_text, text = text.split("-", 1)
        days = int(day_text)
    parts = [int(item) for item in text.split(":")]
    if len(parts) == 3:
        hours, minutes, seconds = parts
    elif len(parts) == 2:
        hours = 0
        minutes, seconds = parts
    else:
        hours = 0
        minutes = 0
        seconds = parts[0]
    return (((days * 24) + hours) * 60 + minutes) * 60 + seconds


def parse_frontend_source_sets(path: Path) -> Dict[str, List[str]]:
    mapping: Dict[str, List[str]] = {}
    current: Optional[str] = None
    for raw_line in path.read_text().splitlines():
        line = raw_line.rstrip()
        if current is not None:
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            continued = stripped.endswith("\\")
            token_line = stripped[:-1].strip() if continued else stripped
            if token_line:
                mapping[current].extend(token_line.split())
            if not continued:
                current = None
            continue

        match = re.match(r"^FRONTEND_OBJ_BASENAMES_([^ ]+)\s*:=\s*(.*)$", line)
        if not match:
            continue
        current = match.group(1)
        mapping[current] = []
        remainder = match.group(2).strip()
        if remainder:
            continued = remainder.endswith("\\")
            token_line = remainder[:-1].strip() if continued else remainder
            if token_line:
                mapping[current].extend(token_line.split())
            if not continued:
                current = None
    return mapping


def parse_pa38_layout(path: Path) -> tuple[List[str], Dict[str, str]]:
    checkpoints: List[str] = []
    stage_to_checkpoint: Dict[str, str] = {}
    for raw_line in path.read_text().splitlines():
        line = raw_line.strip()
        if not line:
            continue
        if line.startswith("CHECKPOINTS = "):
            checkpoints = line.split("=", 1)[1].strip().split()
            continue
        match = re.match(r"^PA38_CHECKPOINT_FOR_(pa\d+) = (\S+)$", line)
        if match:
            stage_to_checkpoint[match.group(1)] = match.group(2)
    if not checkpoints:
        raise SystemExit("failed to parse CHECKPOINTS from pa38/Makefile")
    return checkpoints, stage_to_checkpoint


def capture_processes() -> List[ProcessInfo]:
    output = run_capture(["ps", "-axo", "pid=,ppid=,etime=,command="])
    processes: List[ProcessInfo] = []
    for line in output.splitlines():
        line = line.rstrip()
        if not line:
            continue
        match = re.match(r"^\s*(\d+)\s+(\d+)\s+(\S+)\s+(.*)$", line)
        if not match:
            continue
        pid = int(match.group(1))
        ppid = int(match.group(2))
        elapsed_seconds = parse_elapsed_text(match.group(3))
        command = match.group(4)
        try:
            argv = shlex.split(command)
        except ValueError:
            argv = command.split()
        processes.append(ProcessInfo(pid=pid,
                                     ppid=ppid,
                                     elapsed_seconds=elapsed_seconds,
                                     command=command,
                                     argv=argv))
    return processes


def is_make_process(process: ProcessInfo) -> bool:
    if not process.argv:
        return False
    exe = os.path.basename(process.argv[0])
    return exe == "make" or exe.endswith("/make")


def parse_make_invocation(argv: Sequence[str]) -> tuple[List[str], Dict[str, str]]:
    targets: List[str] = []
    assignments: Dict[str, str] = {}
    i = 1
    while i < len(argv):
        token = argv[i]
        if token in MAKE_OPTIONS_WITH_ARGS and i + 1 < len(argv):
            i += 2
            continue
        if any(token.startswith(option + "=") for option in MAKE_OPTIONS_WITH_ARGS if option.startswith("--")):
            i += 1
            continue
        if token.startswith("-"):
            i += 1
            continue
        if "=" in token and not token.startswith("="):
            key, value = token.split("=", 1)
            assignments[key] = value
        else:
            targets.append(token)
        i += 1
    return targets, assignments


def target_scope(target: str,
                 checkpoints: Sequence[str],
                 stage_to_checkpoint: Dict[str, str]) -> tuple[List[str], str]:
    if not target:
        return list(checkpoints), "all checkpoints"
    if target in ("all", "ladder", "test", "preservation", "inception", "compare-inception", "bitcmp"):
        return list(checkpoints), "all checkpoints"

    compare_match = re.match(r"^compare-(.+)-inception$", target)
    if compare_match:
        checkpoint = compare_match.group(1)
        return [checkpoint], checkpoint

    if target.endswith("-inception"):
        return [target[:-len("-inception")]], target[:-len("-inception")]
    if target.endswith("-self"):
        return [target[:-len("-self")]], target[:-len("-self")]

    if target.startswith("test-through-"):
        base = target[len("test-through-"):]
        checkpoint = stage_to_checkpoint.get(base, base)
        return list(checkpoints[:checkpoints.index(checkpoint) + 1]), f"through {checkpoint}"

    if target.startswith("through-"):
        base = target[len("through-"):]
        checkpoint = stage_to_checkpoint.get(base, base)
        return list(checkpoints[:checkpoints.index(checkpoint) + 1]), f"through {checkpoint}"

    if target.startswith("test-"):
        base = target[len("test-"):]
        if base.endswith("-nobuild"):
            base = base[:-len("-nobuild")]
        checkpoint = stage_to_checkpoint.get(base, base)
        return [checkpoint], checkpoint

    checkpoint = stage_to_checkpoint.get(target, target)
    if checkpoint in checkpoints:
        return [checkpoint], checkpoint
    return list(checkpoints), "all checkpoints"


def infer_flavor(target: str, assignments: Dict[str, str]) -> str:
    if "PA38_COMPILER_FLAVOR" in assignments and assignments["PA38_COMPILER_FLAVOR"]:
        return assignments["PA38_COMPILER_FLAVOR"]
    if target.endswith("-inception") or target in ("inception", "compare-inception", "bitcmp") or target.startswith("compare-"):
        return "inception"
    cxx = assignments.get("CXX", "")
    if cxx in ("../dev/cppgm++", str((REPO_ROOT / "dev" / "cppgm++").resolve())):
        return "selfhost"
    return "host"


def infer_output_suffix(flavor: str, target: str, assignments: Dict[str, str]) -> str:
    if "PA38_OUTPUT_SUFFIX" in assignments and assignments["PA38_OUTPUT_SUFFIX"]:
        return assignments["PA38_OUTPUT_SUFFIX"]
    if flavor == "inception" or target.endswith("-inception"):
        return "-inception"
    return "-self"


def resolve_obj_root_base(assignments: Dict[str, str]) -> Path:
    raw = assignments.get("PA38_OBJ_ROOT_BASE", "../obj/pa38")
    return (PA38_DIR / raw).resolve()


def resolve_bin_root_base(assignments: Dict[str, str], obj_root_base: Path) -> Path:
    raw = assignments.get("PA38_BIN_ROOT_BASE")
    if raw:
        return (PA38_DIR / raw).resolve()
    return (obj_root_base / "bin").resolve()


def resolve_generated_root(assignments: Dict[str, str], obj_root_base: Path) -> Path:
    raw = assignments.get("PA38_GENERATED")
    if raw:
        return (PA38_DIR / raw).resolve()
    return (obj_root_base / "generated").resolve()


def resolve_cxx_dep(raw: Optional[str]) -> Optional[Path]:
    if not raw:
        return None
    if raw.startswith(("./", "../", "/")):
        return resolve_pa38_path(raw)
    return None


def infer_cxx_dep(assignments: Dict[str, str],
                  flavor: str,
                  bin_root_base: Path) -> Optional[Path]:
    if flavor == "inception" and assignments.get("PA38_COMPILER_FLAVOR") != "inception":
        return bin_root_base / "selfhost" / "cppgm++-self"
    explicit = resolve_cxx_dep(assignments.get("CXX"))
    if explicit is not None:
        return explicit
    if flavor == "selfhost":
        return resolve_pa38_path("../dev/cppgm++")
    if flavor == "inception":
        return bin_root_base / "selfhost" / "cppgm++-self"
    return None


def build_spec_from_process(process: ProcessInfo,
                            checkpoints: Sequence[str],
                            stage_to_checkpoint: Dict[str, str]) -> BuildSpec:
    targets, assignments = parse_make_invocation(process.argv)
    target = targets[0] if targets else "all"
    obj_root_base = resolve_obj_root_base(assignments)
    bin_root_base = resolve_bin_root_base(assignments, obj_root_base)
    generated_root = resolve_generated_root(assignments, obj_root_base)
    flavor = infer_flavor(target, assignments)
    output_suffix = infer_output_suffix(flavor, target, assignments)
    test_runner = assignments.get("CPPGM_TEST_RUNNER", "1") != "0"
    cxx_dep = infer_cxx_dep(assignments, flavor, bin_root_base)
    scope, scope_label = target_scope(target, checkpoints, stage_to_checkpoint)
    return BuildSpec(target=target,
                     obj_root_base=obj_root_base,
                     bin_root_base=bin_root_base,
                     generated_root=generated_root,
                     flavor=flavor,
                     output_suffix=output_suffix,
                     test_runner=test_runner,
                     cxx_dep=cxx_dep,
                     checkpoints=scope,
                     scope_label=scope_label)


def manual_build_spec(args: argparse.Namespace,
                      checkpoints: Sequence[str],
                      stage_to_checkpoint: Dict[str, str]) -> Optional[BuildSpec]:
    if not args.target:
        return None
    obj_root_base = (REPO_ROOT / args.obj_root_base).resolve() if args.obj_root_base else (REPO_ROOT / "obj" / "pa38")
    bin_root_base = obj_root_base / "bin"
    generated_root = (REPO_ROOT / args.generated_root).resolve() if args.generated_root else (obj_root_base / "generated").resolve()
    flavor = args.flavor or "selfhost"
    output_suffix = args.output_suffix or ("-inception" if flavor == "inception" else "-self")
    cxx_dep = resolve_cxx_dep(args.cxx) if args.cxx else infer_cxx_dep({}, flavor, bin_root_base)
    scope, scope_label = target_scope(args.target, checkpoints, stage_to_checkpoint)
    return BuildSpec(target=args.target,
                     obj_root_base=obj_root_base,
                     bin_root_base=bin_root_base,
                     generated_root=generated_root,
                     flavor=flavor,
                     output_suffix=output_suffix,
                     test_runner=(not args.no_test_runner),
                     cxx_dep=cxx_dep,
                     checkpoints=scope,
                     scope_label=scope_label)


def process_tree(processes: Sequence[ProcessInfo]) -> Dict[int, List[int]]:
    tree: Dict[int, List[int]] = {}
    for process in processes:
        tree.setdefault(process.ppid, []).append(process.pid)
    return tree


def descendants(root_pid: int, tree: Dict[int, List[int]]) -> Set[int]:
    result: Set[int] = set()
    stack = [root_pid]
    while stack:
        current = stack.pop()
        for child in tree.get(current, []):
            if child in result:
                continue
            result.add(child)
            stack.append(child)
    return result


def checkpoint_entry_path(object_root: Path, checkpoint: str, test_runner: bool) -> Path:
    suffix = "-runner" if test_runner else "-plain"
    return object_root / checkpoint / "release" / f"{checkpoint}{suffix}.o"


def runner_object_path(object_root: Path, test_runner: bool) -> Path:
    suffix = "-enabled" if test_runner else "-disabled"
    return object_root / "shared" / "release" / f"test_runner{suffix}.o"


def binary_path(bin_root: Path, checkpoint: str, output_suffix: str) -> Path:
    return bin_root / f"{checkpoint}{output_suffix}"


def shared_object_stem(stem: str) -> str:
    # pa38/Makefile uses $(basename $(notdir <source>)) for shared objects.
    return Path(stem).name


def shared_object_path(object_root: Path, stem: str) -> Path:
    return object_root / "shared" / "release" / f"{shared_object_stem(stem)}.o"


def shared_depfile_path(object_root: Path, stem: str) -> Path:
    return object_root / "shared" / "release" / ".d" / f"{shared_object_stem(stem)}.d"


def checkpoint_entry_depfile_path(object_root: Path, checkpoint: str, test_runner: bool) -> Path:
    suffix = "-runner" if test_runner else "-plain"
    return object_root / checkpoint / "release" / ".d" / f"{checkpoint}{suffix}.d"


def runner_depfile_path(object_root: Path, test_runner: bool) -> Path:
    suffix = "-enabled" if test_runner else "-disabled"
    return object_root / "shared" / "release" / ".d" / f"test_runner{suffix}.d"


def shared_source_path(stem: str) -> Path:
    return REPO_ROOT / "dev" / "src" / f"{stem}.cpp"


def checkpoint_entry_source_path(checkpoint: str) -> Path:
    return REPO_ROOT / "dev" / f"{checkpoint}.cpp"


def runner_source_path() -> Path:
    return REPO_ROOT / "dev" / "src" / "test_runner.cpp"


def builtin_host_config_path(spec: BuildSpec) -> Path:
    return spec.generated_root / "cppgm_builtin_host_config.h"


def extract_source_arg(argv: Sequence[str]) -> str:
    for item in reversed(argv):
        if item.endswith(".cpp") or item.endswith(".c"):
            return item
    return ""


def arg_after(argv: Sequence[str], flag: str) -> str:
    for index, item in enumerate(argv):
        if item == flag and index + 1 < len(argv):
            return argv[index + 1]
    return ""


def resolve_pa38_path(raw: str) -> Path:
    path = Path(raw)
    if path.is_absolute():
        return path
    return (PA38_DIR / path).resolve()


def split_make_words(text: str) -> List[str]:
    words: List[str] = []
    current: List[str] = []
    escaped = False
    for char in text:
        if escaped:
            current.append(char)
            escaped = False
            continue
        if char == "\\":
            escaped = True
            continue
        if char.isspace():
            if current:
                words.append("".join(current))
                current = []
            continue
        current.append(char)
    if escaped:
        current.append("\\")
    if current:
        words.append("".join(current))
    return words


def parse_depfile(path: Path) -> List[Path]:
    try:
        text = path.read_text()
    except OSError:
        return []

    text = re.sub(r"\\\r?\n", " ", text)
    rules: List[tuple[List[str], List[str]]] = []
    for line in text.splitlines():
        if ":" not in line:
            continue
        targets_text, deps_text = line.split(":", 1)
        targets = split_make_words(targets_text)
        deps = split_make_words(deps_text)
        rules.append((targets, deps))

    phony_targets = {
        target
        for targets, deps in rules
        if not deps
        for target in targets
    }
    for _, deps in rules:
        if not deps:
            continue
        result: List[Path] = []
        for dep in deps:
            path = resolve_pa38_path(dep)
            if path.exists() or dep not in phony_targets:
                result.append(path)
        return result
    return []


def unique_paths(paths: Iterable[Path]) -> List[Path]:
    result: List[Path] = []
    seen: Set[Path] = set()
    for path in paths:
        resolved = path.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        result.append(resolved)
    return result


def object_dependencies(primary_source: Path,
                        depfile: Path,
                        extra_dependencies: Sequence[Path]) -> List[Path]:
    return unique_paths([primary_source, *parse_depfile(depfile), *extra_dependencies])


def compiler_dependencies(spec: BuildSpec) -> List[Path]:
    return [spec.cxx_dep] if spec.cxx_dep is not None else []


def output_is_current(output: Path,
                      dependencies: Sequence[Path],
                      active_outputs: Set[Path]) -> bool:
    output = output.resolve()
    if output in active_outputs:
        return False
    try:
        output_mtime = output.stat().st_mtime
    except OSError:
        return False

    for dependency in dependencies:
        try:
            dependency_mtime = dependency.stat().st_mtime
        except OSError:
            return False
        if dependency_mtime > output_mtime:
            return False
    return True


def shared_object_is_current(spec: BuildSpec,
                             object_root: Path,
                             source_stem: str,
                             active_outputs: Set[Path]) -> bool:
    extra = compiler_dependencies(spec)
    if shared_object_stem(source_stem) == "preprocessor":
        extra = [*extra, builtin_host_config_path(spec)]
    dependencies = object_dependencies(shared_source_path(source_stem),
                                       shared_depfile_path(object_root, source_stem),
                                       extra)
    return output_is_current(shared_object_path(object_root, source_stem),
                             dependencies,
                             active_outputs)


def checkpoint_entry_is_current(spec: BuildSpec,
                                object_root: Path,
                                checkpoint: str,
                                active_outputs: Set[Path]) -> bool:
    dependencies = object_dependencies(checkpoint_entry_source_path(checkpoint),
                                       checkpoint_entry_depfile_path(object_root, checkpoint, spec.test_runner),
                                       compiler_dependencies(spec))
    return output_is_current(checkpoint_entry_path(object_root, checkpoint, spec.test_runner),
                             dependencies,
                             active_outputs)


def runner_object_is_current(spec: BuildSpec,
                             object_root: Path,
                             active_outputs: Set[Path]) -> bool:
    dependencies = object_dependencies(runner_source_path(),
                                       runner_depfile_path(object_root, spec.test_runner),
                                       compiler_dependencies(spec))
    return output_is_current(runner_object_path(object_root, spec.test_runner),
                             dependencies,
                             active_outputs)


def checkpoint_binary_is_current(spec: BuildSpec,
                                 object_root: Path,
                                 bin_root: Path,
                                 checkpoint: str,
                                 shared_stems: Sequence[str],
                                 shared_current: Dict[str, bool],
                                 entry_current: bool,
                                 runner_current: bool,
                                 active_outputs: Set[Path]) -> bool:
    if not entry_current:
        return False
    if spec.test_runner and not runner_current:
        return False
    if any(not shared_current[stem] for stem in shared_stems):
        return False

    dependencies = [
        checkpoint_entry_path(object_root, checkpoint, spec.test_runner),
        *(shared_object_path(object_root, stem) for stem in shared_stems),
    ]
    if spec.test_runner:
        dependencies.append(runner_object_path(object_root, spec.test_runner))
    return output_is_current(binary_path(bin_root, checkpoint, spec.output_suffix),
                             dependencies,
                             active_outputs)


def active_tasks_for_build(root_pid: Optional[int],
                           spec: BuildSpec,
                           processes: Sequence[ProcessInfo],
                           proc_by_pid: Dict[int, ProcessInfo],
                           tree: Dict[int, List[int]]) -> List[ActiveTask]:
    object_root = spec.obj_root_base / spec.flavor
    bin_root = spec.bin_root_base / spec.flavor
    tasks: List[ActiveTask] = []

    candidate_pids: Iterable[int]
    if root_pid is None:
        candidate_pids = [process.pid for process in processes]
    else:
        candidate_pids = descendants(root_pid, tree)

    for pid in sorted(candidate_pids):
        process = proc_by_pid[pid]
        argv = process.argv
        if not argv:
            continue
        output = arg_after(argv, "-o")
        output_path = resolve_pa38_path(output) if output else None
        source = extract_source_arg(argv)

        if output and output_path is not None:
            if object_root in output_path.parents and "-c" in argv:
                label = Path(source).name if source else output_path.name
                detail = str(Path(source)) if source else str(output_path)
                tasks.append(ActiveTask(phase="compile",
                                        label=label,
                                        detail=detail,
                                        elapsed_seconds=process.elapsed_seconds,
                                        output_path=output_path.resolve()))
                continue
            if bin_root in output_path.parents and "-c" not in argv:
                final_output_path = binary_path(bin_root, output_path.name, spec.output_suffix)
                tasks.append(ActiveTask(phase="link",
                                        label=output_path.name,
                                        detail=str(output_path),
                                        elapsed_seconds=process.elapsed_seconds,
                                        output_path=final_output_path.resolve()))
                continue

        if is_make_process(process):
            targets, _ = parse_make_invocation(argv)
            for target in targets:
                if target.startswith("test-"):
                    tasks.append(ActiveTask(phase="test",
                                            label=target,
                                            detail=process.command,
                                            elapsed_seconds=process.elapsed_seconds))
                    break
    return tasks


def build_view(spec: BuildSpec,
               root_pid: Optional[int],
               root_command: str,
               source_sets: Dict[str, List[str]],
               processes: Sequence[ProcessInfo],
               proc_by_pid: Dict[int, ProcessInfo],
               tree: Dict[int, List[int]]) -> BuildView:
    object_root = spec.obj_root_base / spec.flavor
    bin_root = spec.bin_root_base / spec.flavor
    active_tasks = active_tasks_for_build(root_pid, spec, processes, proc_by_pid, tree)
    active_outputs = {
        task.output_path.resolve()
        for task in active_tasks
        if task.output_path is not None
    }

    shared_sources: Dict[str, str] = {}
    checkpoint_shared_stems: Dict[str, List[str]] = {}
    for checkpoint in spec.checkpoints:
        direct_sources = {
            shared_object_stem(source_stem): source_stem
            for source_stem in source_sets[checkpoint]
        }
        checkpoint_shared_stems[checkpoint] = sorted(direct_sources)
        for stem, source_stem in direct_sources.items():
            shared_sources.setdefault(stem, source_stem)

    shared_current = {
        stem: shared_object_is_current(spec, object_root, source_stem, active_outputs)
        for stem, source_stem in shared_sources.items()
    }
    entry_current = {
        checkpoint: checkpoint_entry_is_current(spec, object_root, checkpoint, active_outputs)
        for checkpoint in spec.checkpoints
    }
    runner_current = (
        runner_object_is_current(spec, object_root, active_outputs)
        if spec.test_runner
        else True
    )
    binary_current: Dict[str, bool] = {}
    for checkpoint in spec.checkpoints:
        binary_current[checkpoint] = checkpoint_binary_is_current(
            spec,
            object_root,
            bin_root,
            checkpoint,
            checkpoint_shared_stems[checkpoint],
            shared_current,
            entry_current[checkpoint],
            runner_current,
            active_outputs,
        )

    checkpoint_statuses: List[CheckpointStatus] = []
    for checkpoint in spec.checkpoints:
        direct_shared = checkpoint_shared_stems[checkpoint]
        direct_done = sum(1 for stem in direct_shared if shared_current[stem])
        checkpoint_statuses.append(
            CheckpointStatus(
                name=checkpoint,
                shared_done=direct_done,
                shared_total=len(direct_shared),
                entry_done=entry_current[checkpoint],
                binary_done=binary_current[checkpoint],
            )
        )

    shared_total = len(shared_sources)
    shared_done = sum(1 for stem in shared_sources if shared_current[stem])
    entry_total = len(spec.checkpoints)
    entry_done = sum(1 for checkpoint in spec.checkpoints if entry_current[checkpoint])
    runner_total = 1 if spec.test_runner else 0
    runner_done = 1 if spec.test_runner and runner_current else 0
    binary_total = len(spec.checkpoints)
    binary_done = sum(1 for checkpoint in spec.checkpoints if binary_current[checkpoint])

    return BuildView(
        build_id=str(root_pid) if root_pid is not None else "manual",
        spec=spec,
        object_root=object_root,
        bin_root=bin_root,
        shared_total=shared_total,
        shared_done=shared_done,
        entry_total=entry_total,
        entry_done=entry_done,
        runner_total=runner_total,
        runner_done=runner_done,
        binary_total=binary_total,
        binary_done=binary_done,
        checkpoint_statuses=checkpoint_statuses,
        active_tasks=active_tasks,
        root_pid=root_pid,
        root_elapsed_seconds=proc_by_pid[root_pid].elapsed_seconds if root_pid is not None and root_pid in proc_by_pid else None,
        root_command=root_command,
    )


def discover_builds(processes: Sequence[ProcessInfo],
                    checkpoints: Sequence[str],
                    stage_to_checkpoint: Dict[str, str]) -> List[BuildSpec]:
    builds: List[BuildSpec] = []
    for process in processes:
        if not is_make_process(process):
            continue
        argv = process.argv
        if "-C" not in argv:
            continue
        try:
            c_index = argv.index("-C")
        except ValueError:
            continue
        if c_index + 1 >= len(argv):
            continue
        if argv[c_index + 1] != "pa38":
            continue
        builds.append(build_spec_from_process(process, checkpoints, stage_to_checkpoint))
    return builds


def discover_build_processes(processes: Sequence[ProcessInfo]) -> List[ProcessInfo]:
    roots: List[ProcessInfo] = []
    make_pids = {process.pid for process in processes if is_make_process(process)}
    for process in processes:
        if not is_make_process(process):
            continue
        argv = process.argv
        if "-C" not in argv:
            continue
        try:
            c_index = argv.index("-C")
        except ValueError:
            continue
        if c_index + 1 >= len(argv) or argv[c_index + 1] != "pa38":
            continue
        if process.ppid in make_pids:
            continue
        roots.append(process)
    return roots


def progress_bar(done: int, total: int, width: int = 28) -> str:
    if total <= 0:
        return "[" + ("-" * width) + "]"
    filled = int(round((done / float(total)) * width))
    filled = max(0, min(width, filled))
    return "[" + ("#" * filled) + ("." * (width - filled)) + "]"


def color_for_ratio(done: int, total: int) -> str:
    if total <= 0:
        return Color.BLUE
    ratio = done / float(total)
    if ratio >= 1.0:
        return Color.GREEN
    if ratio >= 0.5:
        return Color.YELLOW
    return Color.BLUE


def format_elapsed(seconds: Optional[int]) -> str:
    if seconds is None:
        return "n/a"
    hours, remainder = divmod(seconds, 3600)
    minutes, secs = divmod(remainder, 60)
    if hours:
        return f"{hours:d}:{minutes:02d}:{secs:02d}"
    return f"{minutes:02d}:{secs:02d}"


def format_path(path: Path) -> str:
    try:
        return str(path.relative_to(REPO_ROOT))
    except ValueError:
        return str(path)


def render_build(view: BuildView, use_color: bool) -> List[str]:
    total_units = view.shared_total + view.entry_total + view.runner_total + view.binary_total
    done_units = view.shared_done + view.entry_done + view.runner_done + view.binary_done
    total_bar = style(progress_bar(done_units, total_units),
                      color_for_ratio(done_units, total_units),
                      enabled=use_color)
    shared_bar = style(progress_bar(view.shared_done, view.shared_total),
                       color_for_ratio(view.shared_done, view.shared_total),
                       enabled=use_color)
    entry_bar = style(progress_bar(view.entry_done, view.entry_total),
                      color_for_ratio(view.entry_done, view.entry_total),
                      enabled=use_color)
    lines = [
        style(f"Build {view.build_id}: {view.spec.target}", Color.BOLD, Color.CYAN, enabled=use_color),
        f"  scope: {view.spec.scope_label}",
        f"  flavor: {style(view.spec.flavor, Color.MAGENTA, enabled=use_color)}   obj: {format_path(view.object_root)}",
        f"  elapsed: {style(format_elapsed(view.root_elapsed_seconds), Color.BOLD, enabled=use_color)}",
        f"  total:  {total_bar} {done_units}/{total_units}",
        f"  shared: {shared_bar} {view.shared_done}/{view.shared_total}",
        f"  entry:  {entry_bar} {view.entry_done}/{view.entry_total}",
    ]
    if view.runner_total:
        runner_bar = style(progress_bar(view.runner_done, view.runner_total),
                           color_for_ratio(view.runner_done, view.runner_total),
                           enabled=use_color)
        lines.append(f"  runner: {runner_bar} {view.runner_done}/{view.runner_total}")
    bin_bar = style(progress_bar(view.binary_done, view.binary_total),
                    color_for_ratio(view.binary_done, view.binary_total),
                    enabled=use_color)
    lines.append(f"  bins:   {bin_bar} {view.binary_done}/{view.binary_total}")

    frontier = next((status for status in view.checkpoint_statuses
                     if not (status.binary_done and status.entry_done and status.shared_done == status.shared_total)), None)
    if frontier is None:
        lines.append(f"  frontier: {style('complete', Color.GREEN, Color.BOLD, enabled=use_color)}")
    else:
        lines.append(
            "  frontier: "
            f"{style(frontier.name, Color.YELLOW, Color.BOLD, enabled=use_color)} "
            f"(shared {frontier.shared_done}/{frontier.shared_total}, "
            f"entry {'yes' if frontier.entry_done else 'no'}, "
            f"bin {'yes' if frontier.binary_done else 'no'})"
        )

    if len(view.checkpoint_statuses) > 1:
        lines.append("  checkpoints:")
        for status in view.checkpoint_statuses:
            state = "done" if (status.binary_done and status.entry_done and status.shared_done == status.shared_total) else "work"
            state_text = style(state, Color.GREEN if state == "done" else Color.YELLOW, enabled=use_color)
            lines.append(
                f"    {state_text} {status.name:<12} "
                f"shared {status.shared_done:>2}/{status.shared_total:<2} "
                f"entry {'Y' if status.entry_done else 'N'} "
                f"bin {'Y' if status.binary_done else 'N'}"
            )

    if view.active_tasks:
        compile_tasks = sorted((task for task in view.active_tasks if task.phase == "compile"),
                               key=lambda task: (task.elapsed_seconds or 0, task.label),
                               reverse=True)
        other_tasks = sorted((task for task in view.active_tasks if task.phase != "compile"),
                             key=lambda task: (task.phase, task.label))
        if compile_tasks:
            lines.append(style(f"  active compiles ({len(compile_tasks)}):", Color.BLUE, Color.BOLD, enabled=use_color))
            for task in compile_tasks:
                elapsed_text = style(format_elapsed(task.elapsed_seconds), Color.DIM, enabled=use_color)
                lines.append(f"    - {style(task.label, Color.BLUE, enabled=use_color)}  {elapsed_text}")
        if other_tasks:
            lines.append(style("  active other:", Color.MAGENTA, Color.BOLD, enabled=use_color))
            for task in other_tasks:
                elapsed_text = style(format_elapsed(task.elapsed_seconds), Color.DIM, enabled=use_color)
                lines.append(
                    f"    - {style(task.phase, Color.MAGENTA, enabled=use_color)}: "
                    f"{task.label}  {elapsed_text}"
                )
    else:
        lines.append(f"  active: {style('idle', Color.DIM, enabled=use_color)}")
    return lines


def render_screen(views: Sequence[BuildView],
                  interval: float,
                  use_color: bool,
                  stale: bool = False) -> str:
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    lines = [style(f"PA38 Build Watch  {now}  refresh={interval:.1f}s",
                   Color.BOLD,
                   Color.CYAN,
                   enabled=use_color), ""]
    if stale:
        lines.append(style("No active pa38 build detected; showing last captured snapshot.",
                           Color.YELLOW,
                           enabled=use_color))
        lines.append("")
    if not views:
        lines.append("No active pa38 build detected.")
        lines.append("Pass --target/--obj-root-base to watch a completed or detached tree manually.")
        return "\n".join(lines)

    for index, view in enumerate(views):
        if index:
            lines.extend(["", style("-" * 72, Color.DIM, enabled=use_color), ""])
        lines.extend(render_build(view, use_color))
    return "\n".join(lines)


def collect_views(args: argparse.Namespace,
                  source_sets: Dict[str, List[str]],
                  checkpoints: Sequence[str],
                  stage_to_checkpoint: Dict[str, str]) -> List[BuildView]:
    processes = capture_processes()
    proc_by_pid = {process.pid: process for process in processes}
    tree = process_tree(processes)

    manual = manual_build_spec(args, checkpoints, stage_to_checkpoint)
    views: List[BuildView] = []
    if manual is not None:
        views.append(build_view(manual,
                                root_pid=None,
                                root_command="manual",
                                source_sets=source_sets,
                                processes=processes,
                                proc_by_pid=proc_by_pid,
                                tree=tree))
        return views

    for process in discover_build_processes(processes):
        if args.pid and process.pid != args.pid:
            continue
        spec = build_spec_from_process(process, checkpoints, stage_to_checkpoint)
        views.append(build_view(spec,
                                root_pid=process.pid,
                                root_command=process.command,
                                source_sets=source_sets,
                                processes=processes,
                                proc_by_pid=proc_by_pid,
                                tree=tree))
    views.sort(key=lambda item: (item.root_pid is None, item.build_id))
    return views


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Watch pa38 build progress, object counts, and active compile files."
    )
    parser.add_argument("--interval", type=float, default=1.0,
                        help="refresh interval in seconds (default: 1.0)")
    parser.add_argument("--once", action="store_true",
                        help="print one snapshot and exit")
    parser.add_argument("--pid", type=int,
                        help="watch one active top-level 'make -C pa38' pid")
    parser.add_argument("--target",
                        help="manual target to watch even if no active make process exists")
    parser.add_argument("--obj-root-base",
                        help="manual PA38_OBJ_ROOT_BASE relative to repo root")
    parser.add_argument("--generated-root",
                        help="manual PA38_GENERATED relative to repo root")
    parser.add_argument("--flavor", choices=["selfhost", "inception", "host"],
                        help="manual compiler flavor")
    parser.add_argument("--cxx",
                        help="manual CXX used for timestamp dependency checks")
    parser.add_argument("--output-suffix",
                        help="manual PA38_OUTPUT_SUFFIX, default inferred from flavor")
    parser.add_argument("--no-test-runner", action="store_true",
                        help="manual mode only: treat test runner object as disabled")
    parser.add_argument("--color", choices=["auto", "always", "never"], default="auto",
                        help="ANSI color mode (default: auto)")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source_sets = parse_frontend_source_sets(FRONTEND_SOURCE_SETS)
    checkpoints, stage_to_checkpoint = parse_pa38_layout(PA38_MAKEFILE)
    use_color = color_enabled(args.color)
    last_views: List[BuildView] = []

    while True:
        current_views = collect_views(args, source_sets, checkpoints, stage_to_checkpoint)
        stale = False
        if current_views:
            last_views = current_views
            views = current_views
        elif last_views:
            stale = True
            views = last_views
        else:
            views = []

        output = render_screen(
            views,
            args.interval,
            use_color,
            stale=stale,
        )
        if not args.once and sys.stdout.isatty():
            sys.stdout.write("\033[2J\033[H")
        sys.stdout.write(output)
        sys.stdout.write("\n")
        sys.stdout.flush()
        if args.once:
            return 0
        time.sleep(args.interval)


if __name__ == "__main__":
    raise SystemExit(main())

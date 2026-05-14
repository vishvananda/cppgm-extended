#!/usr/bin/env python3

import argparse
import concurrent.futures
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


DEFAULT_BUILD_ROOT = Path("/tmp/cppgm-pa-fast-self")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Build a minimal PA frontend with ./dev/cppgm++ and run that PA's tests.",
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--pa", required=True, help="Assignment directory, e.g. pa1")
    parser.add_argument("--compiler", default="./dev/cppgm++")
    parser.add_argument("--host-cxx", default=os.environ.get("CPPGM_HOST_CXX", "clang++"))
    parser.add_argument("--build-root", default=str(DEFAULT_BUILD_ROOT))
    parser.add_argument("--jobs", type=int, default=max(os.cpu_count() or 1, 1))
    parser.add_argument("--build-only", action="store_true")
    parser.add_argument("--clean", action="store_true")
    return parser.parse_args()


def parse_target(pa_makefile: Path) -> str:
    pattern = re.compile(r"^TARGET\s*=\s*(\S+)\s*$")
    for line in pa_makefile.read_text().splitlines():
        match = pattern.match(line)
        if match:
            return match.group(1)
    raise SystemExit(f"Unable to determine TARGET from {pa_makefile}")


def parse_frontend_source_sets(path: Path) -> dict:
    mapping = {}
    lines = path.read_text().splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        match = re.match(r"^FRONTEND_OBJ_BASENAMES_(\S+)\s*:=\s*(.*)$", line)
        if not match:
            i += 1
            continue
        target = match.group(1)
        tail = match.group(2).strip()
        basenames = []
        if tail and tail != "\\":
            basenames.extend(token for token in tail.replace("\\", " ").split() if token)
        i += 1
        while i < len(lines) and lines[i].startswith("\t"):
            chunk = lines[i].replace("\\", " ").strip()
            if chunk:
                basenames.extend(chunk.split())
            i += 1
        mapping[target] = basenames
    return mapping


def nm_symbols(path: Path) -> tuple[set[str], set[str]]:
    proc = subprocess.run(
        ["nm", "-P", str(path)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    defs = set()
    und = set()
    for line in proc.stdout.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        symbol = parts[0]
        kind = parts[1]
        if kind == "U":
            und.add(symbol)
        else:
            defs.add(symbol)
    return defs, und


def build_provider_map(repo_root: Path) -> dict[str, set[str]]:
    provider_map: dict[str, set[str]] = {}
    objdir = repo_root / "obj" / "release"
    for path in sorted(objdir.glob("*.o")):
        if path.name in {"test_runner.o", "test_runner_enabled.o"}:
            continue
        defs, _ = nm_symbols(path)
        stem = path.stem
        for symbol in defs:
            provider_map.setdefault(symbol, set()).add(stem)
    return provider_map


def compile_one(compiler: Path,
                repo_root: Path,
                source: Path,
                output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(compiler),
        "-c",
        "-I",
        str((repo_root / "dev" / "src").resolve()),
        "-o",
        str(output),
        str(source),
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
        raise RuntimeError(
            f"compile failed for {source}\n"
            f"command: {' '.join(cmd)}\n"
            f"stdout:\n{proc.stdout}\n"
            f"stderr:\n{proc.stderr}"
        )


def build_frontend(repo_root: Path,
                   pa: str,
                   target: str,
                   basenames: list[str],
                   provider_map: dict[str, set[str]],
                   compiler: Path,
                   host_cxx: str,
                   build_root: Path,
                   jobs: int) -> Path:
    build_dir = build_root / pa
    obj_dir = build_dir / "obj"
    build_dir.mkdir(parents=True, exist_ok=True)
    obj_dir.mkdir(parents=True, exist_ok=True)

    entry_obj = obj_dir / f"{target}-entry.o"
    compile_one(compiler,
                repo_root,
                repo_root / "dev" / f"{target}.cpp",
                entry_obj)

    selected = set(basenames)
    compiled = set()

    while True:
        pending = sorted(selected - compiled)
        if pending:
            with concurrent.futures.ThreadPoolExecutor(max_workers=max(jobs, 1)) as executor:
                futures = [
                    executor.submit(
                        compile_one,
                        compiler,
                        repo_root,
                        repo_root / "dev" / "src" / f"{stem}.cpp",
                        obj_dir / f"{stem}.o",
                    )
                    for stem in pending
                ]
                for future in concurrent.futures.as_completed(futures):
                    future.result()
            compiled.update(pending)

        unresolved = set()
        all_objects = [entry_obj] + [obj_dir / f"{stem}.o" for stem in sorted(compiled)]
        for path in all_objects:
            _, und = nm_symbols(path)
            unresolved.update(und)

        added = False
        for symbol in sorted(unresolved):
            providers = provider_map.get(symbol)
            if not providers or len(providers) != 1:
                continue
            provider = next(iter(providers))
            if provider in selected:
                continue
            selected.add(provider)
            added = True
        if not added:
            break

    binary = build_dir / f"{target}-self-fast"
    link_cmd = [
        host_cxx,
        "-std=gnu++11",
        "-I",
        str((repo_root / "dev" / "src").resolve()),
        "-o",
        str(binary),
        str(entry_obj),
    ] + [str(obj_dir / f"{stem}.o") for stem in sorted(compiled)]
    proc = subprocess.run(
        link_cmd,
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        raise RuntimeError(
            f"link failed for {pa}\n"
            f"command: {' '.join(link_cmd)}\n"
            f"stdout:\n{proc.stdout}\n"
            f"stderr:\n{proc.stderr}"
        )
    return binary


def run_step(cmd: list[str], cwd: Path) -> None:
    proc = subprocess.run(cmd, cwd=str(cwd), check=False)
    if proc.returncode != 0:
        raise RuntimeError(f"command failed ({proc.returncode}): {' '.join(cmd)}")


def run_tests(repo_root: Path, pa: str, target: str, binary: Path) -> None:
    pa_dir = repo_root / pa
    staged_binary = pa_dir / f"{target}-self-fast"
    if staged_binary.exists() or staged_binary.is_symlink():
        staged_binary.unlink()
    staged_binary.symlink_to(binary)
    suites = ["tests"]
    course_suite = pa_dir / "course" / pa
    if course_suite.exists():
        suites.append(f"course/{pa}")
    for suite in suites:
        run_step(["perl", "scripts/run_all_tests.pl", staged_binary.name, "my", suite], pa_dir)
        run_step(["perl", "scripts/compare_results.pl", "ref", "my", suite], pa_dir)


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    pa = args.pa
    pa_dir = repo_root / pa
    if not pa_dir.is_dir():
        raise SystemExit(f"Missing assignment directory: {pa_dir}")

    compiler = (repo_root / args.compiler).resolve() if not Path(args.compiler).is_absolute() else Path(args.compiler)
    if not compiler.exists():
        raise SystemExit(f"Missing compiler binary: {compiler}")

    build_root = Path(args.build_root).resolve()
    if args.clean:
        shutil.rmtree(build_root / pa, ignore_errors=True)

    target = parse_target(pa_dir / "Makefile")
    closure_map = parse_frontend_source_sets(repo_root / "dev" / "frontend_source_sets.mk")
    provider_map = build_provider_map(repo_root)
    if target not in closure_map:
        raise SystemExit(f"No frontend source set found for target {target}")

    binary = build_frontend(
        repo_root=repo_root,
        pa=pa,
        target=target,
        basenames=closure_map[target],
        provider_map=provider_map,
        compiler=compiler,
        host_cxx=args.host_cxx,
        build_root=build_root,
        jobs=args.jobs,
    )
    print(f"built {binary}")

    if not args.build_only:
        run_tests(repo_root, pa, target, binary)
        print(f"{pa}: tests passed")

    return 0


if __name__ == "__main__":
    sys.exit(main())

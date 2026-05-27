#!/usr/bin/env python3

from pathlib import Path
import re


def split_flag_string(text: str):
    if not text:
        return []
    return [item for item in re.split(r"\s+", text.strip()) if item]


def source_list(repo_root: Path, frontend: str, layout: str, test_runner: bool):
    files = sorted((repo_root / "dev" / "src").glob("*.cpp"))
    files.append(repo_root / frontend)
    if layout == "pa38-selfhost" and test_runner:
        runner = repo_root / "dev" / "src" / "test_runner.cpp"
        if runner not in files:
            files.append(runner)
    return files


def pa38_object_path(build_root: Path, relative: Path, stem: str, test_runner: bool):
    if relative == Path("dev/src/test_runner.cpp"):
        suffix = "-enabled" if test_runner else "-disabled"
        return build_root / "shared" / "release" / f"test_runner{suffix}.o"

    if relative.parts[:2] == ("dev", "src") and relative.suffix == ".cpp":
        return build_root / "shared" / "release" / f"{stem}.o"

    if len(relative.parts) == 2 and relative.parts[0] == "dev" and relative.suffix == ".cpp":
        suffix = "runner" if test_runner else "plain"
        return build_root / stem / "release" / f"{stem}-{suffix}.o"

    raise SystemExit(f"pa38-selfhost layout does not know how to place {relative}")


def depfile_path_for_object(obj: Path):
    return obj.parent / ".d" / (obj.stem + ".d")


def object_path(repo_root: Path,
                build_dir: Path,
                index: int,
                src: Path,
                layout: str,
                test_runner: bool):
    if layout == "pa38-selfhost":
        relative = src.relative_to(repo_root)
        return pa38_object_path(build_dir, relative, src.stem, test_runner)
    return build_dir / (f"{index:03d}-{src.stem}.o")


def compile_command(repo_root: Path,
                    compiler: str,
                    src: Path,
                    obj: Path,
                    layout: str,
                    test_runner: bool,
                    stdlib_flags: str):
    relative = src.relative_to(repo_root)
    cmd = [str((repo_root / compiler).resolve())]

    if layout == "pa38-selfhost":
        cmd.extend(["-std=gnu++11", "-Wall", "-O3"])
        cmd.extend(split_flag_string(stdlib_flags))
        depfile = depfile_path_for_object(obj)
        depfile.parent.mkdir(parents=True, exist_ok=True)
        if relative == Path("dev/src/test_runner.cpp"):
            if test_runner:
                cmd.append("-DTEST_RUNNER_ENABLE")
        elif len(relative.parts) == 2 and relative.parts[0] == "dev":
            if test_runner:
                cmd.append("-Dmain=test_runner_real_main")
        cmd.extend(["-MMD", "-MP", "-MF", str(depfile)])

    cmd.extend(["-I", "dev/src", "-o", str(obj), "-c", str(relative)])
    return cmd

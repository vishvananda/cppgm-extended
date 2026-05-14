#!/usr/bin/env python3

import argparse
import os
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent
TESTS = ROOT / "tests"


def read_validation_kind(path: pathlib.Path) -> str:
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.startswith("// VALIDATION:"):
                return line.split(":", 1)[1].strip()
    raise RuntimeError(f"missing VALIDATION marker in {path}")


def clang_command(test: pathlib.Path, output: pathlib.Path):
    cxx = os.environ.get("CXX", "clang++")
    flags = os.environ.get("CXXFLAGS", "-std=c++11 -Wall -Wextra").split()
    return [cxx, *flags, str(test), "-o", str(output)]


def run_one(test: pathlib.Path) -> tuple[bool, str]:
    kind = read_validation_kind(test)
    with tempfile.TemporaryDirectory(prefix="clang-template-validation-") as td:
        exe = pathlib.Path(td) / "a.out"
        cmd = clang_command(test, exe)
        compile_result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if kind == "compile-fail":
            ok = compile_result.returncode != 0
            detail = "rejected" if ok else "unexpectedly compiled"
            return ok, detail

        if compile_result.returncode != 0:
            return False, "compile failed"

        if kind == "compile-pass":
            return True, "compiled"

        if kind == "run-pass":
            run_result = subprocess.run([str(exe)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            ok = run_result.returncode == 0
            detail = "ran" if ok else f"runtime exit {run_result.returncode}"
            return ok, detail

        raise RuntimeError(f"unknown validation kind {kind} in {test}")


def collect_tests(args) -> list[pathlib.Path]:
    if args.tests:
        return [pathlib.Path(t).resolve() for t in args.tests]
    return sorted(TESTS.glob("*.cpp"))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("tests", nargs="*")
    args = parser.parse_args()

    tests = collect_tests(args)
    failures = 0
    for test in tests:
      ok, detail = run_one(test)
      rel = test.relative_to(ROOT)
      status = "PASS" if ok else "FAIL"
      print(f"{status} {rel} [{detail}]")
      if not ok:
          failures += 1
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

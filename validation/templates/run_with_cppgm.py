#!/usr/bin/env python3

import argparse
import os
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent
TESTS = ROOT / "tests"
DEFAULT_CPPGM = ROOT.parents[1] / "dev" / "cppgm++"


def read_validation_kind(path: pathlib.Path) -> str:
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.startswith("// VALIDATION:"):
                return line.split(":", 1)[1].strip()
    raise RuntimeError(f"missing VALIDATION marker in {path}")


def cppgm_path() -> str:
    return os.environ.get("CPPGM", str(DEFAULT_CPPGM))


def run_one(test: pathlib.Path) -> tuple[bool, str]:
    kind = read_validation_kind(test)
    cppgm = cppgm_path()
    with tempfile.TemporaryDirectory(prefix="cppgm-template-validation-") as td:
        out = pathlib.Path(td) / "a.o"
        cmd = [cppgm, "-c", "-o", str(out), str(test)]

        compile_result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        if kind == "compile-fail":
            ok = compile_result.returncode != 0
            detail = "rejected" if ok else "unexpectedly compiled"
            return ok, detail

        if compile_result.returncode != 0:
            first_error = ""
            for line in compile_result.stderr.splitlines():
                if line.strip():
                    first_error = line.strip()
                    break
            return False, first_error or "compile failed"

        if kind == "compile-pass":
            return True, "compiled"

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

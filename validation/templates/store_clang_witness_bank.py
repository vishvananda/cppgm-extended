#!/usr/bin/env python3

import argparse
import json
import os
import pathlib
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parent
TESTS = ROOT / "tests"
DEFAULT_OUTPUT = ROOT / "witness" / "clang"
DEFAULT_CLANG = pathlib.Path.home() / "llvm-project-template-metrics-20260416" / \
    "build-clang-template-trace" / "bin" / "clang"
DEFAULT_LIBCXX = pathlib.Path("/usr/local/opt/llvm/include/c++/v1")
DEFAULT_SDK = pathlib.Path("/Library/Developer/CommandLineTools/SDKs/MacOSX26.sdk")


def read_validation_kind(path: pathlib.Path) -> str:
    with path.open("r", encoding="utf-8") as handle:
        for line in handle:
            if line.startswith("// VALIDATION:"):
                return line.split(":", 1)[1].strip()
    raise RuntimeError(f"missing VALIDATION marker in {path}")


def collect_tests(args: argparse.Namespace) -> list[pathlib.Path]:
    if args.tests:
        return [pathlib.Path(test).resolve() for test in args.tests]
    return sorted(TESTS.glob("*.cpp"))


def clang_command(clang: pathlib.Path,
                  libcxx: pathlib.Path,
                  sdk: pathlib.Path,
                  test: pathlib.Path) -> list[str]:
    return [
        str(clang),
        "-std=c++11",
        "-fsyntax-only",
        "-nostdinc++",
        "-isystem",
        str(libcxx),
        "-isysroot",
        str(sdk),
        "-x",
        "c++",
        str(test),
    ]


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate and store patched-Clang template witness JSON for the validation bank.")
    parser.add_argument("tests", nargs="*")
    parser.add_argument("--output-dir", default=str(DEFAULT_OUTPUT))
    parser.add_argument("--clang", default=str(DEFAULT_CLANG))
    parser.add_argument("--libcxx", default=str(DEFAULT_LIBCXX))
    parser.add_argument("--sdk", default=str(DEFAULT_SDK))
    args = parser.parse_args()

    output_dir = pathlib.Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    failures = 0
    for test in collect_tests(args):
        rel = test.relative_to(ROOT)
        stem = test.stem
        kind = read_validation_kind(test)
        witness_path = output_dir / f"{stem}.json"
        meta_path = output_dir / f"{stem}.meta.json"
        with tempfile.TemporaryDirectory(prefix="clang-template-witness-bank-") as td:
            td_path = pathlib.Path(td)
            temp_witness = td_path / "witness.json"
            env = os.environ.copy()
            env["CLANG_TEMPLATE_WITNESS_JSON"] = "1"
            env["CLANG_TEMPLATE_WITNESS_OUTPUT"] = str(temp_witness)
            env["CLANG_TEMPLATE_WITNESS_STRIP_PREFIX"] = str(ROOT.parents[1]) + "/"
            command = clang_command(pathlib.Path(args.clang),
                                    pathlib.Path(args.libcxx),
                                    pathlib.Path(args.sdk),
                                    test)
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    text=True, env=env)
            meta = {
                "test": str(rel),
                "validation_kind": kind,
                "command": command,
                "exit_code": result.returncode,
                "compile_succeeded": result.returncode == 0,
                "witness_generated": temp_witness.exists(),
                "stdout": result.stdout,
                "stderr": result.stderr,
            }
            meta_path.write_text(json.dumps(meta, indent=2, sort_keys=True) + "\n",
                                 encoding="utf-8")
            if temp_witness.exists():
                witness_path.write_text(temp_witness.read_text(encoding="utf-8"),
                                        encoding="utf-8")
            elif witness_path.exists():
                witness_path.unlink()

            status = "PASS"
            detail = "stored"
            if kind == "compile-fail":
                detail = "expected compile-fail; metadata stored"
            elif result.returncode != 0:
                status = "FAIL"
                detail = "compile failed"
                failures += 1
            elif not temp_witness.exists():
                status = "FAIL"
                detail = "missing witness output"
                failures += 1

            print(f"{status} {rel} [{detail}]")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

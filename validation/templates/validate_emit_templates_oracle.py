#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
from typing import Dict, List


ROOT = pathlib.Path(__file__).resolve().parent
TESTS = ROOT / "tests"
DEFAULT_CPPGM = ROOT.parents[1] / "dev" / "cppgm++"
DEFAULT_CLANG_DIR = ROOT / "witness" / "clang"
DEFAULT_CPPGM_DIR = ROOT / "witness" / "cppgm_emit"
DEFAULT_REPORT_DIR = ROOT / "witness" / "reports"


def load_module(path: pathlib.Path, name: str):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


COMPARE = load_module(ROOT.parents[1] / "dev" / "compare_template_witness.py",
                      "compare_template_witness")


def read_validation_kind(path: pathlib.Path) -> str:
    with path.open("r", encoding="utf-8") as handle:
        for line in handle:
            if line.startswith("// VALIDATION:"):
                return line.split(":", 1)[1].strip()
    raise RuntimeError(f"missing VALIDATION marker in {path}")


def collect_tests(args: argparse.Namespace) -> List[pathlib.Path]:
    if args.tests:
        return [pathlib.Path(test).resolve() for test in args.tests]
    return sorted(TESTS.glob("*.cpp"))


def run_emit_templates_witness(script: pathlib.Path,
                               cppgm: pathlib.Path,
                               test: pathlib.Path,
                               json_output: pathlib.Path,
                               text_output: pathlib.Path,
                               allow_failure: bool) -> subprocess.CompletedProcess:
    command = [
        sys.executable,
        str(script),
        "--cppgm",
        str(cppgm),
        "--input",
        str(test),
        "--output",
        str(json_output),
        "--emit-text-output",
        str(text_output),
    ]
    if allow_failure:
        command.append("--allow-failure")
    return subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate cppgm++ --emit-templates against stored patched-Clang witness output.")
    parser.add_argument("tests", nargs="*")
    parser.add_argument("--cppgm", default=str(DEFAULT_CPPGM))
    parser.add_argument("--clang-dir", default=str(DEFAULT_CLANG_DIR))
    parser.add_argument("--cppgm-dir", default=str(DEFAULT_CPPGM_DIR))
    parser.add_argument("--report-dir", default=str(DEFAULT_REPORT_DIR))
    parser.add_argument("--compare-selected-decl-location", action="store_true")
    args = parser.parse_args()

    clang_dir = pathlib.Path(args.clang_dir)
    cppgm_dir = pathlib.Path(args.cppgm_dir)
    report_dir = pathlib.Path(args.report_dir)
    cppgm_json_dir = cppgm_dir / "json"
    cppgm_text_dir = cppgm_dir / "text"
    cppgm_meta_dir = cppgm_dir / "meta"
    cppgm_json_dir.mkdir(parents=True, exist_ok=True)
    cppgm_text_dir.mkdir(parents=True, exist_ok=True)
    cppgm_meta_dir.mkdir(parents=True, exist_ok=True)
    report_dir.mkdir(parents=True, exist_ok=True)

    emit_script = ROOT / "emit_templates_to_witness.py"
    summary: List[Dict] = []
    failures = 0

    for test in collect_tests(args):
        rel = test.relative_to(ROOT)
        stem = test.stem
        kind = read_validation_kind(test)
        clang_meta_path = clang_dir / f"{stem}.meta.json"
        clang_json_path = clang_dir / f"{stem}.json"
        cppgm_json_path = cppgm_json_dir / f"{stem}.json"
        cppgm_text_path = cppgm_text_dir / f"{stem}.txt"
        cppgm_meta_path = cppgm_meta_dir / f"{stem}.meta.json"

        run_result = run_emit_templates_witness(emit_script,
                                                pathlib.Path(args.cppgm),
                                                test,
                                                cppgm_json_path,
                                                cppgm_text_path,
                                                allow_failure=True)
        cppgm_meta = {
            "test": str(rel),
            "validation_kind": kind,
            "script_exit_code": run_result.returncode,
            "stdout": run_result.stdout,
            "stderr": run_result.stderr,
        }
        cppgm_meta_path.write_text(json.dumps(cppgm_meta, indent=2, sort_keys=True) + "\n",
                                   encoding="utf-8")

        if not clang_meta_path.exists():
            record = {
                "test": str(rel),
                "status": "missing-clang-witness",
            }
            summary.append(record)
            print(f"SKIP {rel} [missing clang witness]")
            continue

        clang_meta = json.loads(clang_meta_path.read_text(encoding="utf-8"))
        if kind == "compile-fail" or not clang_meta.get("compile_succeeded"):
            record = {
                "test": str(rel),
                "status": "skipped-no-positive-oracle",
            }
            summary.append(record)
            print(f"SKIP {rel} [no positive clang witness oracle]")
            continue

        if not clang_json_path.exists():
            record = {
                "test": str(rel),
                "status": "missing-clang-json",
            }
            summary.append(record)
            print(f"FAIL {rel} [clang witness json missing]")
            failures += 1
            continue

        if not cppgm_json_path.exists():
            record = {
                "test": str(rel),
                "status": "missing-cppgm-json",
            }
            summary.append(record)
            print(f"FAIL {rel} [cppgm emit witness json missing]")
            failures += 1
            continue

        left = json.loads(clang_json_path.read_text(encoding="utf-8"))
        right = json.loads(cppgm_json_path.read_text(encoding="utf-8"))
        issues, notes = COMPARE.compare_documents(
            left,
            right,
            kind_filters=None,
            symbol_pattern=None,
            compare_decls=args.compare_selected_decl_location)
        status = "match" if not issues else "mismatch"
        record = {
            "test": str(rel),
            "status": status,
            "issues": issues,
            "notes": notes,
        }
        summary.append(record)
        report_path = report_dir / f"{stem}.json"
        report_path.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n",
                               encoding="utf-8")
        if issues:
            failures += 1
            print(f"FAIL {rel} [{len(issues)} mismatches]")
        else:
            print(f"PASS {rel} [oracle match]")

    summary_path = report_dir / "summary.json"
    summary_path.write_text(json.dumps(summary, indent=2, sort_keys=True) + "\n",
                            encoding="utf-8")
    matched = sum(1 for record in summary if record["status"] == "match")
    mismatched = sum(1 for record in summary if record["status"] == "mismatch")
    skipped = sum(1 for record in summary if record["status"].startswith("skipped"))
    print(f"SUMMARY matched={matched} mismatched={mismatched} skipped={skipped}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

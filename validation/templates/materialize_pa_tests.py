#!/usr/bin/env python3

import argparse
import importlib.util
import json
import os
import pathlib
import re
import shlex
import subprocess
import sys
import tempfile
from typing import Iterable, List


ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_CLANG = pathlib.Path.home() / "llvm-project-template-metrics-20260416" / \
    "build-clang-template-trace" / "bin" / "clang"
DEFAULT_LIBCXX = pathlib.Path("/usr/local/opt/llvm/include/c++/v1")
DEFAULT_SDK = pathlib.Path("/Library/Developer/CommandLineTools/SDKs/MacOSX26.sdk")
DEFAULT_PAS = ("pa18", "pa19", "pa21", "pa22")
LINK_SOURCE_RE = re.compile(r"\.t\.\d+$")


def load_emit_module():
    path = ROOT / "validation" / "templates" / "emit_templates_to_witness.py"
    spec = importlib.util.spec_from_file_location("emit_templates_to_witness", path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"unable to load witness renderer from {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


EMIT = load_emit_module()


def write_if_different(path: pathlib.Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_text(encoding="utf-8") == text:
        return
    path.write_text(text, encoding="utf-8")


def remove_if_exists(path: pathlib.Path) -> None:
    if path.exists():
        path.unlink()


def expected_success(test: pathlib.Path) -> bool:
    if LINK_SOURCE_RE.search(test.name):
        return True
    exit_path = pathlib.Path(str(test)[:-2] + ".ref.exit_status")
    if not exit_path.exists():
        return True
    return exit_path.read_text(encoding="utf-8").strip() == "EXIT_SUCCESS"


def is_preproc_test(test: pathlib.Path) -> bool:
    parts = test.relative_to(ROOT).parts
    return len(parts) >= 3 and parts[1] == "tests" and parts[2] == "preproc"


def is_pa36_link_manifest(test: pathlib.Path) -> bool:
    parts = test.relative_to(ROOT).parts
    return (len(parts) >= 3 and parts[0] == "pa36" and
            parts[1] == "tests" and parts[2] == "link" and
            test.name.endswith(".t"))


def link_sources_for_manifest(test: pathlib.Path) -> List[pathlib.Path]:
    return sorted(path for path in test.parent.glob(test.name + ".*")
                  if LINK_SOURCE_RE.search(path.name))


def collect_tests(roots: Iterable[str]) -> List[pathlib.Path]:
    out: List[pathlib.Path] = []
    seen = set()

    def add(test: pathlib.Path) -> None:
        test = test.resolve()
        if test in seen:
            return
        seen.add(test)
        out.append(test)

    for root_text in roots:
        root = (ROOT / root_text).resolve()
        if root.is_file():
            if is_pa36_link_manifest(root):
                for source in link_sources_for_manifest(root):
                    add(source)
            elif root.suffix == ".t" or LINK_SOURCE_RE.search(root.name):
                if not is_preproc_test(root):
                    add(root)
            continue
        for test in sorted(root.rglob("*")):
            if not test.is_file() or is_preproc_test(test):
                continue
            if is_pa36_link_manifest(test):
                continue
            if test.suffix == ".t" or LINK_SOURCE_RE.search(test.name):
                add(test)
    return out


def witness_path_for_test(test: pathlib.Path) -> pathlib.Path:
    if LINK_SOURCE_RE.search(test.name):
        return pathlib.Path(str(test) + ".ref.witness")
    return pathlib.Path(str(test)[:-2] + ".ref.witness")


def test_base_for_aux_files(test: pathlib.Path) -> pathlib.Path:
    if LINK_SOURCE_RE.search(test.name):
        return pathlib.Path(LINK_SOURCE_RE.sub("", str(test)))
    return pathlib.Path(str(test)[:-2])


def read_word_list(path: pathlib.Path) -> List[str]:
    if not path.exists():
        return []
    out: List[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        out.extend(shlex.split(line))
    return out


def clang_command(clang: pathlib.Path,
                  libcxx: pathlib.Path,
                  sdk: pathlib.Path,
                  test: pathlib.Path) -> List[str]:
    test_base = test_base_for_aux_files(test)
    return [
        str(clang),
        "-std=gnu++11",
        "-fsyntax-only",
        "-nostdinc++",
        "-isystem",
        str(libcxx),
        "-isysroot",
        str(sdk),
        *read_word_list(pathlib.Path(str(test_base) + ".compile.flags")),
        "-x",
        "c++",
        str(test),
    ]


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Regenerate adjacent .ref.witness files for Clang-valid PA translation units.")
    parser.add_argument("roots", nargs="*", default=list(DEFAULT_PAS),
                        help="PA roots or individual .t files to regenerate")
    parser.add_argument("--clang", default=str(DEFAULT_CLANG))
    parser.add_argument("--libcxx", default=str(DEFAULT_LIBCXX))
    parser.add_argument("--sdk", default=str(DEFAULT_SDK))
    args = parser.parse_args()

    warnings = 0
    updated = 0
    removed = 0

    for test in collect_tests(args.roots):
        rel = test.relative_to(ROOT)
        ref_witness = witness_path_for_test(test)

        if not expected_success(test):
            if ref_witness.exists():
                ref_witness.unlink()
                removed += 1
                print(f"DROP {rel} [non-success ref exit status]")
            continue

        with tempfile.TemporaryDirectory(prefix="clang-template-ref-") as td:
            td_path = pathlib.Path(td)
            witness_json = td_path / "witness.json"
            env = os.environ.copy()
            env["CLANG_TEMPLATE_WITNESS_JSON"] = "1"
            env["CLANG_TEMPLATE_WITNESS_OUTPUT"] = str(witness_json)
            env["CLANG_TEMPLATE_WITNESS_STRIP_PREFIX"] = str(ROOT) + "/"
            command = clang_command(pathlib.Path(args.clang),
                                    pathlib.Path(args.libcxx),
                                    pathlib.Path(args.sdk),
                                    test)
            result = subprocess.run(command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    text=True,
                                    env=env)

            if result.returncode != 0:
                print(f"WARN {rel} [clang exited {result.returncode}; no witness ref]")
                warnings += 1
                continue
            if not witness_json.exists():
                print(f"WARN {rel} [missing clang witness json]")
                warnings += 1
                continue

            document = json.loads(witness_json.read_text(encoding="utf-8"))
            text = EMIT.render_emit_templates_text(document)
            before = ref_witness.read_text(encoding="utf-8") if ref_witness.exists() else None
            write_if_different(ref_witness, text)
            if before != text:
                updated += 1
                print(f"WRITE {rel}")
            else:
                print(f"KEEP {rel}")

    print(f"SUMMARY updated={updated} removed={removed} warnings={warnings}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

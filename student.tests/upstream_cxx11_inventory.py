#!/usr/bin/env python3
"""Inventory pinned upstream compiler trees without hiding unmaterialized tests.

This records coverage candidates and metadata, not applicability decisions.
Unparsed metadata and unavailable source files stay unresolved. Git blob IDs
pin all inputs, including companions outside the current sparse checkout.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess


PINS = {
    "gcc": "5115c7e447fc07457443df874bf57840e8316d5f",
    "llvm-project": "2078da43e25a4623cab2d0d60decddf709aaea28",
}
SOURCE_SUFFIXES = {".C", ".cc", ".cpp", ".cxx", ".c", ".m", ".mm", ".test", ".ll", ".s", ".S"}
METADATA = re.compile(r"dg-[\w-]+|\b(?:RUN|REQUIRES|UNSUPPORTED|XFAIL|DEFINE|REDEFINE):")


def inventory(repository, root):
    revision = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    if revision != PINS[repository]:
        raise ValueError(f"{repository}: expected {PINS[repository]}, found {revision}")
    prefix = "gcc/testsuite" if repository == "gcc" else "clang/test"
    tree = subprocess.check_output(["git", "-C", str(root), "ls-tree", "-rz", "HEAD", prefix])
    for entry in tree.split(b"\0"):
        if not entry:
            continue
        header, encoded_path = entry.split(b"\t", 1)
        mode, kind, blob = header.decode().split()
        if kind != "blob":
            continue
        path = encoded_path.decode()
        family = path[len(prefix) + 1:].split("/", 1)[0]
        if repository == "gcc" and not (family.startswith("g++") or family == "c-c++-common" or family == "lib"):
            continue
        local = root / path
        materialized = local.is_file()
        source = Path(path).suffix in SOURCE_SUFFIXES
        row = dict(repository=repository, revision=revision, path=path, git_blob=blob, mode=mode,
                   family=family, role="candidate" if source else "support", materialized=materialized,
                   status="unresolved" if source else "support-inventoried")
        if materialized:
            data = local.read_bytes()
            row["sha256"] = hashlib.sha256(data).hexdigest()
            row["directives"] = [dict(line=i, text=line) for i, line in
                                 enumerate(data.decode(errors="replace").splitlines(), 1) if METADATA.search(line)]
            if source:
                row["reason"] = "metadata-and-C++11-applicability-await-validation"
        elif source:
            row["reason"] = "source-not-materialized; applicability-awaits-inspection"
        yield row


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    counts = Counter()
    with (args.output / "inventory.jsonl").open("w") as stream:
        for repository in PINS:
            for row in inventory(repository, args.sources / repository):
                stream.write(json.dumps(row) + "\n")
                counts[(repository, row["role"], row["materialized"])] += 1
    summary = [dict(repository=r, role=k, materialized=m, count=n) for (r, k, m), n in sorted(counts.items())]
    manifest = dict(schema=1, pins=PINS, scope="compiler inputs; applicability unresolved",
                    target="x86_64-linux-gnu", dialects=["c++11", "gnu++11"],
                    optimization_levels=[0, 1, 2, 3], counts=summary,
                    excluded_gcc_families="non-C++ and non-shared families; separate library campaign",
                    library_expansion=["libstdc++-v3/testsuite", "libcxx/test"],
                    other_targets="outside initial Linux scope; require later matrix",
                    inventory_sha256=hashlib.sha256((args.output / "inventory.jsonl").read_bytes()).hexdigest())
    manifest["licenses"] = {
        repo: dict(path=license_path, sha256=hashlib.sha256(
            (args.sources / repo / license_path).read_bytes()).hexdigest())
        for repo, license_path in [("gcc", "COPYING3"), ("llvm-project", "LICENSE.TXT")]
    }
    (args.output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()

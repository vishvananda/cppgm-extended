#!/usr/bin/env python3
"""Execute a conservative GCC dg-do run subset after the acceptance survey.

This is an observational host comparison, not a replacement for DejaGnu.
Only unconditional/C++11 run selectors and std/optimization options qualify.
Requirements, skips, xfails, extra sources/options and dg-final checks exclude
a case. Original source bytes are preserved. Both programs must exit zero and
produce identical stdout/stderr. cppgm objects are linked by g++.
"""
import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import re
import shlex

from upstream_cxx11_probe import invoke


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=12)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    manifest = json.loads((args.probe / "manifest.json").read_text())
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    cases = []
    for line in (args.probe / "results.jsonl").read_text().splitlines():
        case = json.loads(line)
        if case["suite"] != "gcc" or case["host"]["status"] != "accept":
            continue
        text = "\n".join(case["directives"])
        if not re.search(r"\{ dg-do run(?: \{ target c\+\+11(?:_only)? \})? \}", text):
            continue
        if re.search(r"dg-(?:require|skip|xfail|additional|add-options|final|error|output|shouldfail)", text):
            continue
        flags = []
        option_lines = [s for s in case["directives"] if "dg-options" in s]
        for option_line in option_lines:
            match = re.search(r'\{ dg-options "([^"]*)" \}', option_line)
            if not match:
                break
            flags.extend(shlex.split(match[1]))
        else:
            if all(re.fullmatch(r"-std=(?:c|gnu)\+\+(?:11|0x)|-O[0-3s]", f) for f in flags):
                cases.append(case)

    def run(case):
        path = case["host"]["command"][-1]
        directory = output / "logs" / case["path"]
        directory.mkdir(parents=True)
        result = dict(path=case["path"], stages={})
        commands = [
            ("host-build", ["g++", "-std=c++11", "-O0", path, "-o", str(directory / "host.exe")]),
            ("cppgm-build", [manifest["compiler"], "-std=c++11", "-O0", "-c", path, "-o", str(directory / "case.o")]),
            ("host-run", [str(directory / "host.exe")]),
            ("cppgm-link", ["g++", str(directory / "case.o"), "-o", str(directory / "cppgm.exe")]),
            ("cppgm-run", [str(directory / "cppgm.exe")]),
        ]
        for name, command in commands:
            outcome = invoke(command, directory / name, 15)
            result["stages"][name] = outcome
            if outcome["status"] != "accept":
                result["result"] = name + ":" + outcome["status"]
                return result
        equal = all((directory / "host-run" / stream).read_bytes() ==
                    (directory / "cppgm-run" / stream).read_bytes() for stream in ["stdout", "stderr"])
        result["result"] = "match" if equal else "output-mismatch"
        return result

    counts = Counter()
    with (output / "results.jsonl").open("w") as log, ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(run, cases):
            log.write(json.dumps(result) + "\n")
            log.flush()
            counts[result["result"]] += 1
    (output / "summary.json").write_text(json.dumps(counts, indent=2) + "\n")
    print(len(cases), json.dumps(counts, indent=2))


if __name__ == "__main__":
    main()

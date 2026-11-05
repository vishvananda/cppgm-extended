#!/usr/bin/env python3
"""Source-acceptance survey, NOT a DejaGnu/lit conformance runner.

Runs original sources with fixed C++11 flags against their host compiler and
cppgm++. Upstream directives are recorded, not executed. Never counts a shared
rejection as a diagnostic-test pass. Logs and output live outside the sources.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time
from collections import Counter


def invoke(command, directory, timeout):
    directory.mkdir(parents=True, exist_ok=True)
    start = time.monotonic()
    with (directory / "stdout").open("wb") as out, (directory / "stderr").open("wb") as err:
        with subprocess.Popen(command, stdout=out, stderr=err, start_new_session=True) as proc:
            try:
                code = proc.wait(timeout=timeout)
                status = "accept" if code == 0 else "crash" if code < 0 or code == 70 else "reject"
            except subprocess.TimeoutExpired:
                os.killpg(proc.pid, signal.SIGKILL)
                proc.wait()
                code, status = None, "timeout"
    return dict(command=command, status=status, exit=code, seconds=time.monotonic() - start)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sources", type=Path, required=True)
    parser.add_argument("--compiler", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=12)
    parser.add_argument("--timeout", type=float, default=15)
    args = parser.parse_args()
    root, output = args.sources.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    compiler = str(args.compiler.resolve())
    cases = [("gcc", p) for p in sorted((root / "gcc/gcc/testsuite/g++.dg/cpp0x").rglob("*.C"))]
    for lane in ["CXX", "SemaCXX", "Parser", "CodeGenCXX"]:
        for path in sorted((root / "llvm-project/clang/test" / lane).rglob("*.cpp")):
            if re.search(r"-std=(?:c|gnu)\+\+(?:11|0x)\b", path.read_text(errors="replace")):
                cases.append(("clang", path))
    manifest = dict(compiler=compiler, compiler_sha256=hashlib.sha256(Path(compiler).read_bytes()).hexdigest(),
                    timeout=args.timeout, count=len(cases), flags=["-std=c++11", "-fsyntax-only"])
    manifest["host_versions"] = {
        host: subprocess.check_output([host, "--version"], text=True).splitlines()[0]
        for host in ["g++", "clang++"]
    }
    for repo in ["gcc", "llvm-project"]:
        manifest[repo] = subprocess.check_output(["git", "-C", str(root / repo), "rev-parse", "HEAD"], text=True).strip()
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")

    def probe(case):
        suite, path = case
        relative = str(path.relative_to(root))
        text = path.read_text(errors="replace")
        log = output / "logs" / relative
        flags = ["-std=c++11", "-fsyntax-only", str(path)]
        host = invoke(["g++" if suite == "gcc" else "clang++", *flags], log / "host", args.timeout)
        candidate = invoke([compiler, *flags], log / "cppgm", args.timeout)
        directives = [line for line in text.splitlines() if re.search(r"dg-|RUN:|REQUIRES:|UNSUPPORTED:|XFAIL:", line)]
        return dict(suite=suite, path=relative, sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                    directives=directives, host=host, cppgm=candidate)

    counts = Counter()
    with (output / "results.jsonl").open("w") as results:
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
            for i, result in enumerate(pool.map(probe, cases), 1):
                results.write(json.dumps(result) + "\n")
                results.flush()
                counts[(result["suite"], result["host"]["status"], result["cppgm"]["status"])] += 1
                if i % 250 == 0:
                    print(f"{i}/{len(cases)}", flush=True)
    summary = [dict(suite=s, host=h, cppgm=c, count=n) for (s, h, c), n in sorted(counts.items())]
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()

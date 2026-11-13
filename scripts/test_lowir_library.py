#!/usr/bin/env python3
"""Optional PA27/PA33 typed LowIR library contract and execution lane."""
import argparse
import os
from pathlib import Path
import platform
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(args):
    subprocess.run([str(a) for a in args], check=True, cwd=ROOT, timeout=120)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--levels", nargs="+", type=int, choices=range(4), default=[0, 1, 2, 3])
    parser.add_argument("--exact-objects", action="store_true", help="additional solution check, not a student requirement")
    args = parser.parse_args()
    if platform.machine().lower() not in ("x86_64", "amd64"):
        raise SystemExit("the optional library lane requires an x86-64 host")
    cc = shlex.split(os.environ.get("CC", "cc"))
    cxx = shlex.split(os.environ.get("CPPGM_HOST_CXX", os.environ.get("CXX", "g++")))
    stdlib = shlex.split(os.environ.get("CPPGM_STDLIB_FLAGS", ""))
    target = 1 if platform.system() == "Darwin" else 0
    with tempfile.TemporaryDirectory(prefix="cppgm-library-") as directory:
        out = Path(directory)
        source = ROOT / "pa27/tests/lowir-library/client.c"
        run([*cc, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pthread", f"-DCPPGM_TEST_TARGET={target}", "-Idev/src", "-c", source, "-o", out / "client.o"])
        # A C++ producer includes the same public records without implementation headers.
        run([*cxx, "-std=c++11", *stdlib, "-Idev/src", "-c", "pa27/tests/lowir-library/scaffold.cpp", "-o", out / "scaffold.o"])
        run([*cxx, *stdlib, out / "scaffold.o", "-o", out / "scaffold"])
        run([out / "scaffold"])
        for form in ("shared", "static"):
            exe = out / form
            library = ["-Ldev", "-lcppgm_lowir", f"-Wl,-rpath,{ROOT / 'dev'}"] if form == "shared" else ["dev/libcppgm_lowir.a"]
            run([*cxx, *stdlib, out / "client.o", *library, "-pthread", "-o", exe])
            for case in ("bad-operand", "bad-layout", "bad-result", "bad-extra", "bad-type", "bad-kind", "bad-float-kind", "bad-reserved", "bad-conversion", "bad-open", "bad-counts", "bad-recommit"):
                run([exe, out / "unused.o", out / "unused.lowir", 0, case])
            for level in args.levels:
                obj = out / f"{form}-{level}.o"
                text = obj.with_suffix(".lowir")
                run([exe, obj, text, level, "valid"])
                comparison = out / "text.o"
                run(["dev/cppgm++", f"-O{level}", "-c", text, "-o", comparison])
                if args.exact_objects and obj.read_bytes() != comparison.read_bytes():
                    raise AssertionError(f"{form} -O{level}: direct and text objects differ")
                host = ROOT / ("pa27/tests/lowir-library/host.c" if level == 0 else "pa33/tests/lowir-library/host.c")
                for name, object_file in (("typed", obj), ("text", comparison)):
                    run([*cc, host, object_file, "-o", out / name])
                    run([out / name])
                pic = out / "pic.o"
                run([exe, pic, out / "pic.lowir", level, "pic"])
                run([*cc, host, pic, "-o", out / "pic"])
                run([out / "pic"])
                entry = out / "entry.o"
                run([exe, entry, out / "entry.lowir", level, "entry"])
                run([*cc, entry, "-o", out / "entry"])
                run([out / "entry"])
            run([exe, "parallel", out, args.levels[-1]])
            if args.exact_objects:
                mac = out / "mac.o"
                text = out / "mac.lowir"
                run([exe, mac, text, args.levels[-1], "macos"])
                comparison = out / "mac-text.o"
                run(["dev/cppgm++", "--target=x86_64-apple-darwin", f"-O{args.levels[-1]}", "-c", text, "-o", comparison])
                if mac.read_bytes() != comparison.read_bytes():
                    raise AssertionError(f"{form}: direct and text Mach-O objects differ")
    print("LowIR library: static/shared, levels " + ",".join(map(str, args.levels)) + ", execution, malformed input, growth, isolation: PASS")


if __name__ == "__main__":
    main()

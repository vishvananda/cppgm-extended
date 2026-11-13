#!/usr/bin/env python3
"""Build a real C consumer against both library forms and compare CLI objects."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(args):
    subprocess.run([str(a) for a in args], check=True, cwd=ROOT)


def main():
    cc = os.environ.get("CC", "cc")
    cxx = os.environ.get("CPPGM_HOST_CXX", "g++")
    with tempfile.TemporaryDirectory(prefix="cppgm-library-") as directory:
        out = Path(directory)
        source = ROOT / "scripts/tests/lowir_library_client.c"
        run([cc, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pthread", "-Idev/src", "-c", source, "-o", out / "client.o"])
        for form in ("shared", "static"):
            exe = out / form
            library = ["-Ldev", "-lcppgm_lowir", f"-Wl,-rpath,{ROOT / 'dev'}"] if form == "shared" else ["dev/libcppgm_lowir.a"]
            run([cxx, out / "client.o", *library, "-pthread", "-o", exe])
            for case in ("bad-operand", "bad-layout", "bad-result", "bad-extra", "bad-type", "bad-kind", "bad-float-kind", "bad-conversion"):
                run([exe, out / "unused.o", out / "unused.lowir", "0", case])
            for level in range(4):
                obj = out / f"{form}-{level}.o"
                text = obj.with_suffix(".lowir")
                run([exe, obj, text, level, "valid"])
                comparison = out / "text.o"
                run(["dev/cppgm++", f"-O{level}", "-c", text, "-o", comparison])
                if obj.read_bytes() != comparison.read_bytes():
                    raise AssertionError(f"{form} -O{level}: direct and text objects differ")
                host = out / "host.c"
                host.write_text("extern long compute(long); long host_adjust(long x) { return x * 3; }\n"
                                "int main(void) { return compute(0) != 21 || compute(10) != 156 || compute(100) != 14871; }\n")
                run([cc, host, obj, "-o", out / "program"])
                run([out / "program"])
            entry = out / "entry.o"
            entry_text = out / "entry.lowir"
            run([exe, entry, entry_text, 3, "entry"])
            if "role=entry" not in entry_text.read_text():
                raise AssertionError("conventional main did not acquire its entry role")
            run([cc, entry, "-o", out / "entry"])
            run([out / "entry"])
            run([exe, "parallel", out])
            mac = out / "mac.o"
            text = out / "mac.lowir"
            run([exe, mac, text, 3, "macos"])
            comparison = out / "mac-text.o"
            run(["dev/cppgm++", "--target=x86_64-apple-darwin", "-O3", "-c", text, "-o", comparison])
            if mac.read_bytes() != comparison.read_bytes():
                raise AssertionError(f"{form}: direct and text Mach-O objects differ")
    print("LowIR C library: static/shared, O0-O3, text equality, execution, malformed input, isolation: PASS")


if __name__ == "__main__":
    main()

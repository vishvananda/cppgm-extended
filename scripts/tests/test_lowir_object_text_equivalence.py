#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]


def host_cxx():
    return (
        os.environ.get("CPPGM_HOST_CXX")
        or os.environ.get("CXX")
        or "/usr/local/opt/llvm/bin/clang++"
    )


def run(*args, **kwargs):
    kwargs.setdefault("check", True)
    kwargs.setdefault("text", True)
    return subprocess.run(args, **kwargs)


class LowirObjectTextEquivalenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        run(
            "make",
            "-C",
            "dev",
            "cppgm++",
            "lowiropt",
            "cpplink",
            f"CXX={host_cxx()}",
            f"CPPGM_HOST_CXX={host_cxx()}",
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def test_source_object_matches_text_roundtrip_object(self):
        with tempfile.TemporaryDirectory(prefix="lowir-object-text-equiv.") as temp_dir:
            temp = Path(temp_dir)
            src = temp / "probe.cpp"
            src.write_text(
                "template<class T>\n"
                "inline T * idptr(T * p) { return p; }\n"
                "\n"
                "int use(int * p) {\n"
                "  if(!p) {\n"
                "    return 0;\n"
                "  }\n"
                "  return *idptr(p);\n"
                "}\n"
            )

            for debug_level in (0, 1):
                for opt_level in (0, 1):
                    with self.subTest(debug_level=debug_level, opt_level=opt_level):
                        source_object = temp / f"source-g{debug_level}-O{opt_level}.o"
                        roundtrip_object = temp / f"text-g{debug_level}-O{opt_level}.o"
                        source_lowir = temp / f"source-g{debug_level}-O0.lir"
                        prepared_lowir = temp / f"text-g{debug_level}-O{opt_level}.lir"

                        run(
                            str(REPO_ROOT / "dev" / "cppgm++"),
                            "-c",
                            f"-g{debug_level}",
                            f"-O{opt_level}",
                            "-o",
                            str(source_object),
                            str(src),
                            cwd=REPO_ROOT,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                        )

                        run(
                            str(REPO_ROOT / "dev" / "cppgm++"),
                            "--emit-lowir",
                            f"-g{debug_level}",
                            "-O0",
                            "-o",
                            str(source_lowir),
                            str(src),
                            cwd=REPO_ROOT,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                        )

                        run(
                            str(REPO_ROOT / "dev" / "lowiropt"),
                            f"-O{opt_level}",
                            "-o",
                            str(prepared_lowir),
                            str(source_lowir),
                            cwd=REPO_ROOT,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                        )

                        run(
                            str(REPO_ROOT / "dev" / "cpplink"),
                            "-c",
                            "-o",
                            str(roundtrip_object),
                            str(prepared_lowir),
                            cwd=REPO_ROOT,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                        )

                        source_bytes = source_object.read_bytes()
                        roundtrip_bytes = roundtrip_object.read_bytes()
                        if source_bytes != roundtrip_bytes:
                            self.fail(
                                "source object diverged from textual LowIR roundtrip for "
                                f"-g{debug_level} -O{opt_level}: "
                                f"{len(source_bytes)} vs {len(roundtrip_bytes)} bytes"
                            )


if __name__ == "__main__":
    unittest.main()

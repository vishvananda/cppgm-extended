#!/usr/bin/env python3

import os
from pathlib import Path
import shlex
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


def first_line_words(path):
    if not path.exists():
        return []
    text = path.read_text()
    line = text.splitlines()[0] if text.splitlines() else ""
    return shlex.split(line)


def harness_sources(path):
    path = Path(path)
    if path.suffix == ".t":
        candidates = sorted(
            path.parent.glob(path.name + ".*"),
            key=lambda p: int(p.name.rsplit(".", 1)[1])
            if p.name.rsplit(".", 1)[1].isdigit()
            else 10**9,
        )
        sources = [
            p for p in candidates
            if p.name.rsplit(".", 1)[1].isdigit()
        ]
        if sources:
            return sources
    return [path]


def object_summary(path):
    nm = subprocess.run(
        ["nm", "-a", str(path)],
        cwd=REPO_ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    ).stdout
    lines = [line for line in nm.splitlines() if line.strip()]
    return "\n".join(sorted(lines)[:80])


class LowirObjectTextEquivalenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        run(
            "make",
            "-C",
            "dev",
            "cppgm++",
            f"CXX={host_cxx()}",
            f"CPPGM_HOST_CXX={host_cxx()}",
            cwd=REPO_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    def assert_object_roundtrip_matches(self, src, temp, extra_flags=None):
        extra_flags = extra_flags or []
        for debug_flag in ("-g0", "-gline-tables-only"):
            for opt_level in (0, 1):
                with self.subTest(src=str(src), debug_flag=debug_flag, opt_level=opt_level):
                    safe_name = "".join(
                        ch if ch.isalnum() else "_"
                        for ch in str(src.relative_to(REPO_ROOT)
                                      if src.is_relative_to(REPO_ROOT)
                                      else src)
                    )
                    debug_name = debug_flag.replace("-", "").replace("=", "_")
                    source_object = temp / f"source-{safe_name}-{debug_name}-O{opt_level}.o"
                    roundtrip_object = temp / f"roundtrip-{safe_name}-{debug_name}-O{opt_level}.o"
                    common_args = [
                        str(REPO_ROOT / "dev" / "cppgm++"),
                        "-c",
                        *extra_flags,
                        debug_flag,
                        f"-O{opt_level}",
                    ]

                    run(
                        *common_args,
                        "-o",
                        str(source_object),
                        str(src),
                        cwd=REPO_ROOT,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                    )

                    run(
                        *common_args,
                        "--roundtrip-object-lowir",
                        "-o",
                        str(roundtrip_object),
                        str(src),
                        cwd=REPO_ROOT,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                    )

                    source_bytes = source_object.read_bytes()
                    roundtrip_bytes = roundtrip_object.read_bytes()
                    if source_bytes != roundtrip_bytes:
                        self.fail(
                            "source object diverged from textual LowIR roundtrip for "
                            f"{src} {debug_flag} -O{opt_level}: "
                            f"{len(source_bytes)} vs {len(roundtrip_bytes)} bytes\n"
                            "source symbols:\n"
                            f"{object_summary(source_object)}\n"
                            "roundtrip symbols:\n"
                            f"{object_summary(roundtrip_object)}"
                        )

    def test_synthetic_object_emission_survives_lowir_text_roundtrip(self):
        with tempfile.TemporaryDirectory(prefix="lowir-object-text-equiv.") as temp_dir:
            temp = Path(temp_dir)
            src = temp / "probe.cpp"
            src.write_text(
                "template<class T>\n"
                "struct Box {\n"
                "  static T * idptr(T * p) { return p; }\n"
                "};\n"
                "\n"
                "static int fallback = 11;\n"
                "\n"
                "extern \"C\" int use(int * p) {\n"
                "  if(!p) {\n"
                "    p = &fallback;\n"
                "  }\n"
                "  return *Box<int>::idptr(p);\n"
                "}\n"
                "\n"
                "int (*selected)(int *) = &use;\n"
            )
            self.assert_object_roundtrip_matches(src, temp)

    def test_configured_harness_sources_survive_lowir_text_roundtrip(self):
        configured = os.environ.get("CPPGM_LOWIR_OBJECT_ROUNDTRIP_INPUTS", "")
        if not configured.strip():
            self.skipTest("set CPPGM_LOWIR_OBJECT_ROUNDTRIP_INPUTS to add harness sources")

        with tempfile.TemporaryDirectory(prefix="lowir-object-text-equiv.") as temp_dir:
            temp = Path(temp_dir)
            for item in shlex.split(configured):
                test_path = (REPO_ROOT / item).resolve()
                compile_flags = first_line_words(
                    test_path.with_suffix(test_path.suffix + ".compile.flags")
                    if test_path.suffix != ".t"
                    else test_path.with_suffix(".compile.flags"))
                for src in harness_sources(test_path):
                    self.assert_object_roundtrip_matches(src.resolve(), temp, compile_flags)


if __name__ == "__main__":
    unittest.main()

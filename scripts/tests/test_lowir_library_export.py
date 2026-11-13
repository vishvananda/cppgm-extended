#!/usr/bin/env python3
"""Exercise the exported optional library build using only shipped sources."""
import platform
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from test_exported_dev_makefile import exported_dev_makefile, shell_array, REPO_ROOT


class LibraryExportTests(unittest.TestCase):
    def test_c_vocabulary_is_current(self):
        subprocess.run(["python3", "scripts/generate_lowir_c_vocabulary.py", "--check"], cwd=REPO_ROOT, check=True)

    def test_exported_library_stub_builds_without_solution(self):
        script = (REPO_ROOT / "scripts/export_student_repo.sh").read_text()
        with tempfile.TemporaryDirectory(prefix="cppgm-library-export-") as directory:
            root = Path(directory)
            dev = root / "dev"
            dev.mkdir()
            for relative in shell_array(script, "dev_support_files"):
                if relative.endswith(".h"):
                    target = root / relative
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy(REPO_ROOT / relative, target)
            (dev / "Makefile").write_text(exported_dev_makefile())
            shutil.copy(REPO_ROOT / "dev/lowir_library.mk", dev)
            shutil.copy(REPO_ROOT / "dev/lowir_library-scaffold.cpp", dev / "lowir_library.cpp")
            (dev / "frontend_source_sets.mk").write_text("FRONTEND_LIBRARY_SOURCE_IDS_lowir_library :=\n" + "\n".join(
                "FRONTEND_OBJ_BASENAMES_" + tool + " :=" for tool in
                ("abimangle", "pptoken", "posttoken", "ppexpr", "preproc", "cppgm++", "lowir", "lowiropt", "lowir2native")))
            subprocess.run(["make", "-s", "-C", dev, "lowir-library", "-j2", "CPPGM_TEST_RUNNER=0", "CXX=g++"], check=True, capture_output=True, text=True)
            source = root / "client.c"
            source.write_text('#include "lowir_interface.h"\nint main(void) {\n'
                              'CppgmLowir* m = cppgm_lowir_create(CPPGM_LOWIR_ABI_VERSION, 0);\n'
                              'if (m || !cppgm_lowir_error(m)[0]) return 1;\n'
                              'cppgm_lowir_destroy(m); return 0; }\n')
            obj = root / "client.o"
            subprocess.run(["cc", "-std=c11", "-Werror", "-I" + str(dev / "src"), "-c", source, "-o", obj], check=True)
            shared = "libcppgm_lowir.dylib" if platform.system() == "Darwin" else "libcppgm_lowir.so"
            for library in ("libcppgm_lowir.a", shared):
                exe = root / "client"
                subprocess.run(["g++", obj, dev / library, "-Wl,-rpath," + str(dev), "-pthread", "-o", exe], check=True)
                subprocess.run([exe], check=True)
            # Returning to an older object root must relink the shared outputs.
            for obj_root in ("../other-objects", "../obj"):
                before = (dev / "libcppgm_lowir.a").stat().st_mtime_ns
                subprocess.run(["make", "-s", "-C", dev, "lowir-library", "-j2", "CXX=g++", "OBJ=" + obj_root], check=True, capture_output=True)
                self.assertGreater((dev / "libcppgm_lowir.a").stat().st_mtime_ns, before)
            # The optional implementation is independently rebuildable and cleanable.
            subprocess.run(["make", "-s", "-C", dev, "clean-lowir-library", "CXX=g++"], check=True, capture_output=True)
            self.assertFalse((dev / "libcppgm_lowir.a").exists())


if __name__ == "__main__":
    unittest.main()

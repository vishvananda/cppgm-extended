#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess
import tempfile
import time
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]
EXPORT_SCRIPT = REPO_ROOT / "scripts" / "export_student_repo.sh"


def exported_dev_makefile():
    script = EXPORT_SCRIPT.read_text()
    marker = "cat > \"$dest/dev/Makefile\" <<'EOF'\n"
    start = script.index(marker) + len(marker)
    end = script.index("\nEOF\n", start)
    return script[start:end] + "\n"


class ExportedDevMakefileTests(unittest.TestCase):
    def test_identical_relink_preserves_frontend_mtime(self):
        with tempfile.TemporaryDirectory(prefix="exported-dev-makefile.") as temp:
            root = Path(temp)
            dev = root / "dev"
            src = dev / "src"
            src.mkdir(parents=True)
            (dev / "Makefile").write_text(exported_dev_makefile())

            targets = [
                "abimangle", "pptoken", "posttoken", "ctrlexpr", "macro",
                "preproc", "recog", "lowir2cy86", "lowiropt", "lowir2native",
                "cppgm++", "nsdecl", "nsinit", "cy86",
            ]
            source_sets = []
            for target in targets:
                objects = "shared" if target == "pptoken" else ""
                source_sets.append(
                    f"FRONTEND_OBJ_BASENAMES_{target} := {objects}\n"
                )
            (dev / "frontend_source_sets.mk").write_text("".join(source_sets))
            (dev / "pptoken.cpp").write_text("entry-v1\n")
            (src / "shared.cpp").write_text("shared-v1\n")

            compiler = root / "fake-cxx"
            compiler.write_text(
                "#!/usr/bin/env python3\n"
                "import hashlib, pathlib, sys\n"
                "args = sys.argv[1:]\n"
                "out = pathlib.Path(args[args.index('-o') + 1])\n"
                "inputs = [pathlib.Path(a) for a in args if pathlib.Path(a).is_file()]\n"
                "payload = b''.join(path.read_bytes() for path in inputs)\n"
                "out.parent.mkdir(parents=True, exist_ok=True)\n"
                "out.write_bytes(hashlib.sha256(payload).digest())\n"
                "if '-MF' in args:\n"
                "    dep = pathlib.Path(args[args.index('-MF') + 1])\n"
                "    dep.parent.mkdir(parents=True, exist_ok=True)\n"
                "    dep.write_text(str(out) + ':\\n')\n"
            )
            compiler.chmod(0o755)

            command = [
                "make", "-s", "-C", str(dev), "pptoken",
                "CPPGM_TEST_RUNNER=0", f"CXX={compiler}",
            ]
            subprocess.run(command, check=True)
            binary = dev / "pptoken"
            original_mtime = binary.stat().st_mtime_ns
            original_content = binary.read_bytes()

            result = subprocess.run(
                command, check=True, text=True, stdout=subprocess.PIPE
            )
            self.assertNotIn("LINK", result.stdout)
            self.assertEqual(original_mtime, binary.stat().st_mtime_ns)

            time.sleep(0.02)
            os.utime(root / "obj" / "dev" / "shared.o", None)
            result = subprocess.run(
                command, check=True, text=True, stdout=subprocess.PIPE
            )
            self.assertIn("LINK", result.stdout)
            self.assertEqual(original_content, binary.read_bytes())
            self.assertEqual(original_mtime, binary.stat().st_mtime_ns)

            time.sleep(0.02)
            (src / "shared.cpp").write_text("shared-v2\n")
            subprocess.run(command, check=True)
            self.assertNotEqual(original_content, binary.read_bytes())
            self.assertGreater(binary.stat().st_mtime_ns, original_mtime)


if __name__ == "__main__":
    unittest.main()

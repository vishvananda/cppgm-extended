#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


REPO_ROOT = Path(__file__).resolve().parents[2]


def docker_image():
    return os.environ.get("CPPGM_LINUX_DOCKER_IMAGE", "cppgm-clang22-libcxx:latest")


def docker_api_version():
    return os.environ.get("DOCKER_API_VERSION", "1.52")


def run(*args, **kwargs):
    kwargs.setdefault("check", True)
    kwargs.setdefault("text", True)
    return subprocess.run(args, **kwargs)


def docker_env():
    env = os.environ.copy()
    env["DOCKER_API_VERSION"] = docker_api_version()
    return env


def have_docker_image(image):
    try:
            result = subprocess.run(
            ["docker", "image", "inspect", image],
            env=docker_env(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
    except FileNotFoundError:
        return False
    return result.returncode == 0


class LinuxPlatformRecoveryTests(unittest.TestCase):
    def test_pa34_batch_compile_workers_follow_test_jobs(self):
        result = run(
            "make",
            "-C",
            str(REPO_ROOT / "pa34"),
            "-pn",
            env={**os.environ, "CPPGM_TEST_JOBS": "1"},
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.assertIn(
            "CPPGM_HOSTCOMPAT_COMPILE_WORKERS = $(CPPGM_TEST_JOBS)",
            result.stdout,
        )
    def test_host_builtin_runtime_compiles_in_linux_clang22_container(self):
        image = docker_image()
        if not have_docker_image(image):
            self.skipTest(f"docker image not available: {image}")

        with tempfile.TemporaryDirectory(prefix="cppgm-linux-host-runtime.") as temp_dir:
            temp = Path(temp_dir)
            command = [
                "docker",
                "run",
                "--rm",
                "-u",
                f"{os.getuid()}:{os.getgid()}",
                "-v",
                f"{REPO_ROOT}:/cppgm:ro",
                "-v",
                f"{temp}:/work",
                image,
                "bash",
                "-lc",
                (
                    "set -euo pipefail\n"
                    "/usr/bin/clang++-22 -std=gnu++11 -Wall -O3 -stdlib=libc++ "
                    "-DCPPGM_DEFAULT_HOST_CXX='\"/usr/bin/clang++-22\"' "
                    "-I/cppgm/dev/src "
                    "-c -o /work/host_builtin_runtime.o "
                    "/cppgm/dev/src/host_builtin_runtime.cpp\n"
                ),
            ]
            run(
                *command,
                cwd=REPO_ROOT,
                env=docker_env(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            self.assertTrue((temp / "host_builtin_runtime.o").exists())


if __name__ == "__main__":
    unittest.main()

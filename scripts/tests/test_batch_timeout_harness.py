#!/usr/bin/env python3

import errno
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import textwrap
import time
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


def wait_for_file(path: Path, timeout_sec: float) -> str:
    deadline = time.time() + timeout_sec
    while time.time() < deadline:
        if path.exists():
            return path.read_text().strip()
        time.sleep(0.01)
    raise AssertionError(f"timed out waiting for {path}")


def wait_for_pid_exit(pid: int, timeout_sec: float) -> None:
    deadline = time.time() + timeout_sec
    while time.time() < deadline:
        try:
            os.kill(pid, 0)
        except ProcessLookupError:
            return
        except OSError as exc:
            if exc.errno == errno.ESRCH:
                return
            raise
        time.sleep(0.01)
    raise AssertionError(f"pid {pid} still alive after timeout")


class BatchTimeoutHarnessTests(unittest.TestCase):
    def test_test_runner_timeout_kills_process_group(self):
        with tempfile.TemporaryDirectory(prefix="test-runner-batch-timeout.") as temp_dir:
            temp = Path(temp_dir)
            probe = temp / "test_runner_timeout_probe"
            source = temp / "test_runner_timeout_probe.cpp"
            source_obj = temp / "test_runner_timeout_probe.o"
            runner_obj = temp / "test_runner.o"
            pidfile = temp / "descendant.pid"
            stdout_path = temp / "request.stdout"
            stderr_path = temp / "request.stderr"

            source.write_text(
                textwrap.dedent(
                    """
                    #include <csignal>
                    #include <fstream>
                    #include <string>
                    #include <unistd.h>

                    int main(int argc, char ** argv)
                    {
                      if(argc == 3 && std::string(argv[1]) == "--hang-descendant") {
                        signal(SIGTERM, SIG_IGN);
                        const pid_t child = fork();
                        if(child < 0) {
                          return 1;
                        }
                        if(child == 0) {
                          signal(SIGTERM, SIG_IGN);
                          for(;;) {
                            pause();
                          }
                        }
                        std::ofstream pidfile(argv[2]);
                        if(!pidfile) {
                          return 1;
                        }
                        pidfile << child << std::endl;
                        pidfile.close();
                        for(;;) {
                          pause();
                        }
                      }
                      return 0;
                    }
                    """
                )
            )

            run(
                host_cxx(),
                "-std=gnu++11",
                "-Dmain=test_runner_real_main",
                "-I",
                str(REPO_ROOT / "dev" / "src"),
                "-c",
                str(source),
                "-o",
                str(source_obj),
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            run(
                host_cxx(),
                "-std=gnu++11",
                "-DTEST_RUNNER_ENABLE",
                "-I",
                str(REPO_ROOT / "dev" / "src"),
                "-c",
                str(REPO_ROOT / "dev" / "src" / "test_runner.cpp"),
                "-o",
                str(runner_obj),
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            run(
                host_cxx(),
                str(source_obj),
                str(runner_obj),
                "-o",
                str(probe),
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            request = "\t".join(
                [
                    str(stdout_path),
                    str(stderr_path),
                    "-",
                    "CPPGM_BATCH_TIMEOUT_SEC=1",
                    "--hang-descendant",
                    str(pidfile),
                ]
            ) + "\n"

            result = run(
                "env",
                "WRAPPED_BATCH_STDIN=1",
                str(probe),
                "--batch-stdin",
                cwd=REPO_ROOT,
                input=request,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, "124\n")
            self.assertEqual(result.stderr, "")
            self.assertEqual(stderr_path.read_text(), "")

            pid = int(wait_for_file(pidfile, 3.0))
            wait_for_pid_exit(pid, 3.0)

    def test_cppgm_batch_worker_timeout_kills_process_group(self):
        with tempfile.TemporaryDirectory(prefix="perl-batch-timeout.") as temp_dir:
            temp = Path(temp_dir)
            helper = temp / "hang_descendant.py"
            pidfile = temp / "descendant.pid"
            stdout_path = temp / "capture.stdout"
            stderr_path = temp / "capture.stderr"
            driver = temp / "run_capture.pl"

            helper.write_text(
                textwrap.dedent(
                    """
                    #!/usr/bin/env python3
                    import signal
                    import sys
                    import time
                    import os

                    signal.signal(signal.SIGTERM, signal.SIG_IGN)
                    child = os.fork()
                    if child == 0:
                        signal.signal(signal.SIGTERM, signal.SIG_IGN)
                        while True:
                            signal.pause()
                    with open(sys.argv[1], "w") as fh:
                        fh.write(str(child))
                    while True:
                        signal.pause()
                    """
                )
            )
            helper.chmod(0o755)

            driver.write_text(
                textwrap.dedent(
                    f"""
                    #!/usr/bin/env perl
                    use strict;
                    use warnings;
                    use lib {str(REPO_ROOT / "scripts")!r};
                    use CppgmBatchWorker qw(run_command_capture);

                    my $status = run_command_capture(
                      cmd => [{sys.executable!r}, {str(helper)!r}, {str(pidfile)!r}],
                      stdout => {str(stdout_path)!r},
                      stderr => {str(stderr_path)!r},
                      timeout => 1,
                    );
                    print "$status\\n";
                    """
                )
            )
            driver.chmod(0o755)

            result = run(
                "perl",
                str(driver),
                cwd=REPO_ROOT,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stdout, "124\n")
            self.assertEqual(result.stderr, "")

            pid = int(wait_for_file(pidfile, 3.0))
            wait_for_pid_exit(pid, 3.0)


if __name__ == "__main__":
    unittest.main()

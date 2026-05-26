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

    def test_run_all_text_test_timeout_marks_single_test_timeout(self):
        with tempfile.TemporaryDirectory(prefix="run-all-text-timeout.") as temp_dir:
            temp = Path(temp_dir) / "pa3"
            tests = temp / "tests"
            app = temp / "hang_text_test.py"
            test = tests / "hang.t"

            tests.mkdir(parents=True)
            test.write_text("input\n")
            app.write_text(
                "#!/usr/bin/env python3\n"
                "import time\n"
                "\n"
                "while True:\n"
                "    time.sleep(60)\n"
            )
            app.chmod(0o755)

            env = os.environ.copy()
            env["CPPGM_TEXT_TEST_TIMEOUT_SEC"] = "1"
            result = run(
                "perl",
                str(REPO_ROOT / "scripts" / "run_all_tests_common.pl"),
                "text_t",
                str(app),
                "my",
                "tests",
                cwd=temp,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stderr, "")
            self.assertEqual((tests / "hang.my.exit_status").read_text(), "EXIT_TIMEOUT\n")

    def test_run_all_text_test_keeps_later_assignment_output_args(self):
        with tempfile.TemporaryDirectory(prefix="run-all-text-output-args.") as temp_dir:
            temp = Path(temp_dir) / "pa10"
            tests = temp / "tests"
            app = temp / "write_output_arg.py"
            test = tests / "basic.t"

            tests.mkdir(parents=True)
            test.write_text("input\n")
            app.write_text(
                "#!/usr/bin/env python3\n"
                "import sys\n"
                "\n"
                "if len(sys.argv) != 4 or sys.argv[1] != '-o':\n"
                "    sys.exit(2)\n"
                "with open(sys.argv[2], 'w') as fh:\n"
                "    fh.write('generated output\\n')\n"
                "print('stdout log', flush=True)\n"
                "print('stderr log', file=sys.stderr, flush=True)\n"
            )
            app.chmod(0o755)

            result = run(
                "perl",
                str(REPO_ROOT / "scripts" / "run_all_tests_common.pl"),
                "text_t",
                str(app),
                "my",
                "tests",
                cwd=temp,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            self.assertEqual(result.returncode, 0)
            self.assertEqual(result.stderr, "")
            self.assertEqual((tests / "basic.my.exit_status").read_text(), "EXIT_SUCCESS\n")
            self.assertEqual((tests / "basic.my").read_text(), "generated output\n")
            self.assertEqual((tests / "basic.my.stdout").read_text(), "stdout log\nstderr log\n")


if __name__ == "__main__":
    unittest.main()

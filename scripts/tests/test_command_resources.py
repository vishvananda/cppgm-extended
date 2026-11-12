"""Exercise the native exec protocol, actual limits, and cancellation on both hosts."""
import json
import os
from pathlib import Path
import select
import shlex
import shutil
import signal
import subprocess
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]


class CommandResources(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.scratch = tempfile.TemporaryDirectory(prefix='cppgm-resource-tests-')
        cls.root = Path(cls.scratch.name)
        cls.probe = cls.root / 'probe'
        source = cls.root / 'probe.cpp'
        source.write_text(r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h>

int test_runner_real_main(int argc, char** argv) {
    if (argc < 2) return 0;
    if (!strcmp(argv[1], "exit")) return atoi(argv[2]);
    if (!strcmp(argv[1], "echo")) {
        for (int i = 2; i < argc; ++i) {
            printf("%zu:", strlen(argv[i]));
            fwrite(argv[i], 1, strlen(argv[i]), stdout);
        }
        const char* env = getenv("PROBE_VALUE");
        if (env) printf("env=%s", env);
        return 0;
    }
    if (!strcmp(argv[1], "busy")) {
        volatile unsigned count = 0;
        for (;;) ++count;
    }
    if (!strcmp(argv[1], "memory")) {
        for (;;) {
            char* memory = static_cast<char*>(malloc(1024 * 1024));
            if (!memory) return 1;
            memset(memory, 42, 1024 * 1024);
            usleep(1000);
        }
    }
    if (!strcmp(argv[1], "output") || !strcmp(argv[1], "file")) {
        fprintf(stderr, "ERROR: simulated rejection before resource exhaustion\n");
        FILE* output = argc == 3 ? fopen(argv[2], "w") : stdout;
        if (!output) return 1;
        char bytes[4096] = {};
        for (;;) fwrite(bytes, 1, sizeof(bytes), output);
    }
    if (!strcmp(argv[1], "descendants") || !strcmp(argv[1], "detach")) {
        const pid_t child = fork();
        if (child == 0) {
            signal(SIGTERM, SIG_IGN);
            for (;;) pause();
        }
        FILE* ids = fopen(argv[2], "w");
        if (!ids || child < 0) return 1;
        fprintf(ids, "%ld %ld\n", static_cast<long>(getpid()), static_cast<long>(child));
        fclose(ids);
        if (!strcmp(argv[1], "detach")) return 0;
        for (;;) pause();
    }
    if (!strcmp(argv[1], "group-memory")) {
        for (int i = 0; i < 2; ++i) {
            if (fork() == 0) {
                char* memory = static_cast<char*>(malloc(24 * 1024 * 1024));
                if (!memory) return 1;
                memset(memory, 42, 24 * 1024 * 1024);
                for (;;) pause();
            }
        }
        for (;;) pause();
    }
    if (!strcmp(argv[1], "signal")) raise(SIGSEGV);
    if (!strcmp(argv[1], "sleep")) sleep(3);
    if (!strcmp(argv[1], "reject")) {
        fprintf(stderr, "ERROR: expected diagnostic\n");
        return 1;
    }
    if (!strcmp(argv[1], "internal")) {
        fprintf(stderr, "ERROR: internal compiler error\n");
        return 70;
    }
    return 0;
}
''')
        compiler = shlex.split(os.environ.get('CPPGM_HOST_CXX', 'c++'))
        subprocess.run([*compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
                        '-DTEST_RUNNER_ENABLE', str(source),
                        str(ROOT / 'dev/src/support/testing/test_runner.cpp'),
                        '-o', str(cls.probe)], check=True, capture_output=True)

    @classmethod
    def tearDownClass(cls):
        cls.scratch.cleanup()

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(dir=self.root)
        self.path = Path(self.directory.name)
        self.worker = subprocess.Popen([str(self.probe), '--batch-exec-v1'],
                                       env={**os.environ, 'WRAPPED_BATCH_STDIN': '1'},
                                       stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                       stderr=subprocess.PIPE)

    def tearDown(self):
        if self.worker.poll() is None:
            self.worker.stdin.close()
            try:
                self.worker.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.worker.terminate()
                self.worker.wait(timeout=5)
        self.worker.stdin.close()
        self.worker.stdout.close()
        self.worker.stderr.close()
        self.directory.cleanup()

    def request(self, *args, wall=3000, cpu=2, memory=256*1024*1024,
                output=64*1024, env=None):
        strings = [str(self.probe), *map(str, args)]
        fields = ['exec-v1', str(wall), str(cpu), str(memory), str(output)]
        fields.extend(str(p).encode().hex() for p in
                      ('/dev/null', self.path / 'stdout', self.path / 'stderr'))
        fields += [str(len(strings)), *(s.encode().hex() for s in strings)]
        env = env or {}
        fields += [str(len(env))]
        for key, value in env.items():
            fields += [key.encode().hex(), value.encode().hex()]
        return ('\t'.join(fields) + '\n').encode()

    def send(self, request):
        self.worker.stdin.write(request)
        self.worker.stdin.flush()
        self.assertTrue(select.select([self.worker.stdout], [], [], 8)[0], 'worker did not respond')
        return self.worker.stdout.readline().decode().strip().split('\t')[1:]

    def healthy(self):
        self.assertEqual(self.send(self.request('echo', 'ready')), ['0', 'exit'])
        self.assertEqual((self.path / 'stdout').read_bytes(), b'5:ready')

    def assert_gone(self, pid):
        for _ in range(100):
            try:
                os.kill(pid, 0)
            except ProcessLookupError:
                return
            # Container PID 1 may not reap adopted children promptly.
            status = Path(f'/proc/{pid}/stat')
            try:
                zombie = status.exists() and status.read_text().rsplit(')', 1)[1].split()[0] == 'Z'
            except (ProcessLookupError, FileNotFoundError):
                return
            if zombie:
                return
            time.sleep(.02)
        self.fail(f'child {pid} survived its command')

    def wait_ids(self):
        path = self.path / 'ids'
        for _ in range(200):
            if path.exists() and len(path.read_text().split()) == 2:
                return list(map(int, path.read_text().split()))
            time.sleep(.01)
        self.fail('child did not start')

    def test_arguments_and_environment_preserve_bytes_and_empty_values(self):
        args = ['', 'a\tb\nc\r', 'λ☃', 'trailing ']
        self.assertEqual(self.send(self.request('echo', *args, env={'PROBE_VALUE': 'x\ty\nz'})), ['0', 'exit'])
        expected = b''.join(str(len(a.encode())).encode() + b':' + a.encode() for a in args)
        self.assertEqual((self.path / 'stdout').read_bytes(), expected + b'env=x\ty\nz')
        self.healthy()

    def test_malformed_requests_do_not_poison_next_request(self):
        good = self.request('echo', 'ready')
        for bad in (b'exec-v2\n', good.replace(b'exec-v1\t3000', b'exec-v1\t0'),
                    good.replace(b'7265616479', b'00'), good.replace(b'7265616479', b'zz'),
                    good.replace(b'7265616479', b'0'), b'x' * (4*1024*1024 + 1) + b'\n'):
            with self.subTest(length=len(bad)):
                self.assertEqual(self.send(bad), ['125', 'protocol'])
                self.healthy()

    def test_wall_and_cpu_limits(self):
        self.assertEqual(self.send(self.request('sleep', wall=100)), ['124', 'timeout'])
        self.healthy()
        status, kind = self.send(self.request('busy', wall=4000, cpu=1))
        self.assertEqual(kind, 'signal')
        self.assertIn(int(status), (signal.SIGKILL, signal.SIGXCPU))
        self.healthy()

    def test_guest_exit_statuses_are_distinct_from_supervisor_failures(self):
        for status in (1, 70, 124, 125, 126, 127, 200, 255):
            self.assertEqual(self.send(self.request('exit', status)), [str(status), 'exit'])
        missing = self.request('echo').replace(str(self.probe).encode().hex().encode(), b'2f6e6f2d737563682d746f6f6c')
        self.assertEqual(self.send(missing), ['125', 'setup'])
        self.healthy()

    def test_resident_memory_including_descendants(self):
        for mode in ('memory', 'group-memory'):
            with self.subTest(mode=mode):
                self.assertEqual(self.send(self.request(mode, memory=40*1024*1024)), ['125', 'memory'])
                self.healthy()

    def test_output_and_created_files_are_bounded(self):
        for args, path in [(('output',), self.path / 'stdout'),
                           (('file', self.path / 'created'), self.path / 'created')]:
            self.assertEqual(self.send(self.request(*args, output=8192)), ['125', 'output'])
            self.assertLessEqual(path.stat().st_size, 8192)
            self.healthy()

    def test_previous_output_is_not_inspected_before_child_setup(self):
        for name in ('stdout', 'stderr'):
            (self.path / name).write_bytes(b'x' * (64 * 1024))
        fifo = self.path / 'input'
        os.mkfifo(fifo)
        request = self.request('echo', 'ready').replace(b'2f6465762f6e756c6c', str(fifo).encode().hex().encode())
        self.worker.stdin.write(request)
        self.worker.stdin.flush()
        # The child blocks opening stdin before it can truncate either capture.
        # A previous full capture must not terminate this new command.
        self.assertFalse(select.select([self.worker.stdout], [], [], .1)[0])
        descriptor = os.open(fifo, os.O_RDWR | os.O_NONBLOCK)
        try:
            self.assertTrue(select.select([self.worker.stdout], [], [], 3)[0])
            self.assertEqual(self.worker.stdout.readline(), b'exec-v1\t0\texit\n')
        finally:
            os.close(descriptor)
        self.assertEqual((self.path / 'stdout').read_bytes(), b'5:ready')
        self.healthy()

    def test_timeout_kills_descendant_after_leader_exits_on_term(self):
        self.assertEqual(self.send(self.request('descendants', self.path / 'ids', wall=100)), ['124', 'timeout'])
        for pid in self.wait_ids():
            self.assert_gone(pid)
        self.healthy()

    def test_success_cleans_up_descendants(self):
        self.assertEqual(self.send(self.request('detach', self.path / 'ids')), ['0', 'exit'])
        for pid in self.wait_ids():
            self.assert_gone(pid)
        self.healthy()

    def test_cancelling_worker_kills_active_child_group(self):
        self.worker.stdin.write(self.request('descendants', self.path / 'ids'))
        self.worker.stdin.flush()
        ids = self.wait_ids()
        self.worker.terminate()
        self.assertEqual(self.worker.wait(timeout=3), 143)
        for pid in ids:
            self.assert_gone(pid)

    def test_perl_client_never_accepts_resource_internal_or_signal_failures(self):
        code = r'''
use CppgmBatchWorker qw(run_command_capture);
use JSON::PP;
my @rows;
for my $mode (qw(output internal signal reject echo)) {
    my ($status, $error);
    eval { $status = run_command_capture(cmd=>[$ARGV[0],$mode],
        supervisor=>$ARGV[0], stdout=>"$ARGV[1]/out", stderr=>"$ARGV[1]/err",
        timeout=>3, internal_error=>70, limits=>{cpu_seconds=>2,rss_bytes=>268435456,file_bytes=>8192}); };
    push @rows, {status=>$status, error=>"$@"};
}
print encode_json(\@rows);
'''
        result = subprocess.run(['perl', '-I', str(ROOT / 'scripts'), '-e', code,
                                 str(self.probe), str(self.path)], capture_output=True, text=True, timeout=10,
                                env={k: v for k, v in os.environ.items() if k != 'CPPGM_TEST_SUPERVISOR'})
        self.assertEqual(result.returncode, 0, result.stderr)
        rows = json.loads(result.stdout)
        self.assertTrue(all(row['error'] and row['status'] is None for row in rows[:3]))
        self.assertEqual(rows[3:], [{'status': 1, 'error': ''}, {'status': 0, 'error': ''}])


    def test_supervisor_cleanup_preserves_checker_exit_status(self):
        code = r'''
use CppgmBatchWorker qw(run_command_capture);
run_command_capture(cmd=>[$ARGV[0], 'echo'], supervisor=>$ARGV[0],
    stdout=>"$ARGV[1]/out", stderr=>"$ARGV[1]/err", timeout=>3,
    limits=>{cpu_seconds=>2,rss_bytes=>268435456,file_bytes=>8192});
if ($ARGV[2] eq 'die') { die "expected checker failure\n"; }
exit int($ARGV[2]);
'''
        for ending, expected in [('die', None), ('7', 7), ('0', 0)]:
            with self.subTest(ending=ending):
                result = subprocess.run(
                    ['perl', '-I', str(ROOT / 'scripts'), '-e', code,
                     str(self.probe), str(self.path), ending],
                    capture_output=True, text=True, timeout=10,
                    env={k: v for k, v in os.environ.items()
                         if k != 'CPPGM_TEST_SUPERVISOR'})
                if expected is None:
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn('expected checker failure', result.stderr)
                else:
                    self.assertEqual(result.returncode, expected, result.stderr)


class WasmMemoryLimits(unittest.TestCase):
    @unittest.skipUnless(shutil.which('wasmtime') and shutil.which('wasm-tools'), 'pinned Wasm tools not on PATH')
    def test_guest_memory_limit_without_changing_virtual_memory_reservation(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            wat = path / 'grow.wat'
            module = path / 'grow.wasm'
            wat.write_text('''(module (memory 1)
                (func (export "prepare") (param i32))
                (func (export "run") (param i32 i32 i32) (result i32)
                  i32.const 4096 memory.grow))''')
            subprocess.run(['wasm-tools', 'parse', str(wat), '-o', str(module)], check=True, capture_output=True)
            run = subprocess.run(['wasmtime', 'run', '-W', 'max-memory-size=268435456',
                                  '--invoke', 'run', str(module), '0', '1', '17'], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(run.stdout.strip(), '-1')
            engine = os.environ.get('CPPGM_WASM_C_API')
            if engine:
                compiler = shlex.split(os.environ.get('CPPGM_HOST_CXX', 'c++'))
                runner = path / 'runner'
                subprocess.run([*compiler, '-std=c++11', '-O2', '-Wall', '-Wextra', '-Werror',
                                '-I' + str(Path(engine) / 'include'),
                                str(ROOT / 'scripts/wasm_benchmark_runner.cpp'),
                                str(Path(engine) / 'lib/libwasmtime.a'), '-ldl', '-lpthread', '-lm',
                                '-o', str(runner)], check=True, capture_output=True)
                run = subprocess.run([str(runner), str(module), str(module)], input='0 0 1 17\n',
                                     capture_output=True, text=True, timeout=30)
                self.assertEqual(run.returncode, 0, run.stderr)
                rows = [json.loads(line) for line in run.stdout.splitlines()]
                self.assertEqual(rows[-1]['result'], 4294967295)


if __name__ == '__main__':
    unittest.main()

"""Only immutable successful trusted-tool observations may be reused."""
from pathlib import Path
import json
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class WasmCheckCache(unittest.TestCase):
    def run_requests(self, commands, files=None):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, data in (files or {'a.wasm': b'core', 'b.wasm': b'core'}).items():
                (root / name).write_bytes(data)
            program = r'''
use CppgmWasmCheckCache;
use JSON::PP;
my $requests = decode_json($ARGV[0]);
my $cache = CppgmWasmCheckCache->new(validator => 'validator', runtime => 'runtime');
my ($calls, $request, @rows) = (0);
no warnings 'redefine';
local *CppgmWasmCheckCache::run_command_capture = sub {
    my (%options) = @_;
    ++$calls;
    die "infrastructure failed\n" if $request->{die};
    for my $name ('stdout', 'stderr') {
        open(my $file, '>', $options{$name}) or die $!;
        print {$file} "$name:$calls";
        close($file) or die $!;
    }
    return $request->{status} // 0;
};
for my $next (@$requests) {
    $request = $next;
    if ($request->{write}) {
        while (my ($name, $hex) = each %{$request->{write}}) {
            open(my $file, '>:raw', $name) or die $!;
            print {$file} pack('H*', $hex);
            close($file) or die $!;
        }
    }
    my %outcome;
    my %options = (cmd => $request->{cmd}, stdout => 'out', stderr => 'err',
        timeout => 30, internal_error => 70,
        limits => {cpu_seconds => 30, rss_bytes => 2147483648, file_bytes => 67108864},
        outcome => \%outcome, %{$request->{options} // {}});
    my ($status, $error);
    eval { $status = $cache->capture(%options); 1 } or $error = $@;
    my %streams;
    for my $name ('stdout', 'stderr') {
        my $path = $options{$name};
        open(my $file, '<', $path) or next;
        local $/; $streams{$name} = <$file>;
    }
    push @rows, {calls => $calls, status => $status, error => $error, %streams};
}
print encode_json(\@rows);
'''
            result = subprocess.run(['perl', '-I', str(ROOT / 'scripts'), '-e', program,
                                     json.dumps(commands)], cwd=root, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            return json.loads(result.stdout)

    def test_identical_bytes_at_different_paths_replay_both_streams(self):
        rows = self.run_requests([{'cmd': ['validator', 'validate', name],
                                   'options': {'stdout': name + '.out', 'stderr': name + '.err'}}
                                  for name in ('a.wasm', 'b.wasm')])
        self.assertEqual([x['calls'] for x in rows], [1, 1])
        self.assertEqual(rows[1]['stdout'], 'stdout:1')
        self.assertEqual(rows[1]['stderr'], 'stderr:1')

    def test_changed_bytes_flags_and_resource_settings_invalidate(self):
        base = ['validator', 'validate', 'a.wasm']
        rows = self.run_requests([
            {'cmd': base}, {'cmd': base, 'write': {'a.wasm': b'changed'.hex()}},
            {'cmd': ['validator', 'validate', '--features=simd', 'a.wasm']},
            {'cmd': base, 'options': {'timeout': 12}},
            {'cmd': base, 'options': {'limits': {'cpu_seconds': 1, 'rss_bytes': 2048, 'file_bytes': 128}}},
            {'cmd': base, 'options': {'internal_error': 0}},
        ])
        self.assertEqual([x['calls'] for x in rows], list(range(1, 7)))

    def test_failures_and_infrastructure_errors_are_never_cached(self):
        cmd = ['validator', 'validate', 'a.wasm']
        rows = self.run_requests([{'cmd': cmd, 'status': 124}, {'cmd': cmd, 'status': 1},
                                  {'cmd': cmd, 'die': True}, {'cmd': cmd}])
        self.assertEqual([x['calls'] for x in rows], [1, 2, 3, 4])
        self.assertIn('infrastructure failed', rows[2]['error'])

    def test_export_arguments_and_preload_identity_are_part_of_the_key(self):
        def cmd(name, value='17', host='host.wasm', namespace='host'):
            return ['runtime', 'run', '--preload', namespace + '=' + host,
                    '--invoke', 'observe', name, value]
        rows = self.run_requests([
            {'cmd': cmd('a.wasm')}, {'cmd': cmd('b.wasm')},
            {'cmd': cmd('b.wasm', '18')}, {'cmd': cmd('b.wasm', namespace='other')},
            {'cmd': cmd('b.wasm'), 'write': {'host.wasm': b'changed'.hex()}},
        ], {'a.wasm': b'core', 'b.wasm': b'core', 'host.wasm': b'host'})
        self.assertEqual([x['calls'] for x in rows], [1, 1, 2, 3, 4])

    def test_runtime_initialization_and_fuel_modes_are_distinct(self):
        base = ['runtime', 'run', '--invoke', 'observe', 'a.wasm']
        segment = base + ['-O', 'memory-init-cow=n', '-W', 'fuel=1000']
        rows = self.run_requests([{'cmd': base}, {'cmd': segment}, {'cmd': segment},
                                  {'cmd': base + ['-W', 'fuel=100']}])
        self.assertEqual([x['calls'] for x in rows], [1, 2, 2, 3])

    def test_wasi_modules_and_preloads_run_fresh(self):
        for wasi in ('a.wasm', 'host.wasm'):
            with self.subTest(wasi=wasi):
                files = {'a.wasm': b'core', 'host.wasm': b'host'}
                files[wasi] += b'wasi_snapshot_preview1'
                cmd = ['runtime', 'run', '--preload', 'host=host.wasm', '--invoke', 'observe', 'a.wasm']
                rows = self.run_requests([{'cmd': cmd}, {'cmd': cmd}], files)
                self.assertEqual([x['calls'] for x in rows], [1, 2])

    def test_commands_host_state_and_traps_run_fresh(self):
        for extra in ([], ['--invoke', 'observe', '--dir', '.'], ['--invoke', 'observe', '--env', 'X=1']):
            cmd = ['runtime', 'run', *extra, 'a.wasm']
            rows = self.run_requests([{'cmd': cmd}, {'cmd': cmd}])
            self.assertEqual([x['calls'] for x in rows], [1, 2])
        cmd = ['runtime', 'run', '--invoke', 'observe', 'a.wasm']
        rows = self.run_requests([{'cmd': cmd, 'status': 1}, {'cmd': cmd, 'status': 1}])
        self.assertEqual([x['calls'] for x in rows], [1, 2])
        for options in ({'env': {'X': '1'}}, {'stdin': 'a.wasm'}):
            rows = self.run_requests([{'cmd': cmd, 'options': options}] * 2)
            self.assertEqual([x['calls'] for x in rows], [1, 2])

    def test_compiler_invocations_always_run(self):
        cmd = ['compiler', '-c', 'a.wasm']
        rows = self.run_requests([{'cmd': cmd}, {'cmd': cmd}])
        self.assertEqual([x['calls'] for x in rows], [1, 2])


if __name__ == '__main__':
    unittest.main()

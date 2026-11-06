#!/usr/bin/env python3
"""Run one declared C++11 variant with GCC's native dg diagnostic oracle.

Use a fresh output directory per invocation. Native assertions are retained
verbatim, with independent abnormal-exit and capability dispositions. This
maintainer adapter does not establish corpus applicability or conformance.
"""
import argparse
import base64
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import sys

from upstream_dg_proxy import execute, sha256, write_json


EXTRACT = r'''
proc verbose {args} {}
source [file join [lindex $argv 0] utils.exp]
source [file join [lindex $argv 0] dg.exp]
foreach directive [dg-get-options [lindex $argv 1]] {
    set words {}
    foreach word $directive {
        lappend words [binary encode base64 -maxlen 0 [encoding convertto utf-8 $word]]
    }
    puts [join $words \t]
}
'''

DRIVER = r'''
# Standalone installed-compiler configuration: use DejaGnu's native Unix
# loader. GCC's in-build-tree target configuration is not present here.
# Define it before gcc-dg.exp installs its native shouldfail/environment wrapper.
proc g++_load {program args} {
    global cppgm_runtime_proxy
    set ::env(CPPGM_DG_EXECUTABLE) [file normalize $program]
    return [remote_load target $cppgm_runtime_proxy {*}$args]
}
load_lib g++-dg.exp
# Only this native helper discards the requested assembly without inspecting
# it. Other assembly probes must not read a substituted ELF object as text.
rename check_no_compiler_messages_nocache cppgm_native_no_messages
proc check_no_compiler_messages_nocache {args} {
    set had_previous [info exists ::env(CPPGM_DG_PROBE_ORACLE)]
    if {$had_previous} { set previous $::env(CPPGM_DG_PROBE_ORACLE) }
    set ::env(CPPGM_DG_PROBE_ORACLE) no-messages
    try {
        return [cppgm_native_no_messages {*}$args]
    } finally {
        if {$had_previous} {
            set ::env(CPPGM_DG_PROBE_ORACLE) $previous
        } else {
            unset ::env(CPPGM_DG_PROBE_ORACLE)
        }
    }
}
dg-init
set ::env(CPPGM_DG_PHASE) test
set metadata [open $cppgm_metadata w]
foreach directive [dg-get-options $cppgm_case] {
    set words {}
    foreach word $directive {
        lappend words [binary encode base64 -maxlen 0 [encoding convertto utf-8 $word]]
    }
    puts $metadata [join $words \t]
}
close $metadata
dg-test $cppgm_case "-std=$cppgm_dialect" $cppgm_defaults
set ::env(CPPGM_DG_PHASE) cleanup
dg-finish
'''


def tcl(value):
    encoded = base64.b64encode(str(value).encode()).decode()
    return '[encoding convertfrom utf-8 [binary decode base64 {' + encoded + '}]]'


def parse_metadata(text):
    directives = []
    for line in text.splitlines():
        words = [base64.b64decode(value).decode() for value in line.split('\t')]
        directives.append(dict(command=words[0], line=int(words[1]), arguments=words[2:]))
    return directives


def assertions(text):
    return [dict(status=match[1], text=match[2]) for line in text.splitlines()
            if (match := re.match(r'^(PASS|FAIL|XPASS|XFAIL|UNSUPPORTED|UNRESOLVED|ERROR|WARNING):\s*(.*)', line))]


def disposition(native, calls, execution, blockers=()):
    reasons = list(blockers)
    abnormal = []
    for call in calls:
        classification = call['classification']
        if classification in {'started', 'adapter-error'}:
            reasons.append('incomplete or failed adapter invocation')
        elif call.get('phase') == 'test':
            if classification in {'abnormal', 'timeout'}:
                abnormal.append(dict(role=call.get('role'), classification=classification,
                                     incoming=call['incoming']))
            elif classification == 'blocked':
                reasons.append(call['reason'])
    if execution['classification'] in {'timeout', 'abnormal'}:
        return dict(status='harness-abnormal', reasons=[execution['classification']], abnormal=abnormal)
    if abnormal:
        if any(c['role'] == 'runtime' for c in abnormal):
            status = 'runtime-abnormal'
        else:
            status = 'compiler-abnormal' if any(c['role'] == 'test' for c in abnormal) else 'probe-abnormal'
        return dict(status=status, reasons=reasons, abnormal=abnormal)
    if reasons:
        return dict(status='harness-blocked', reasons=sorted(set(reasons)), abnormal=[])
    counts = Counter(item['status'] for item in native)
    if any(counts[key] for key in ['ERROR', 'WARNING']):
        return dict(status='harness-unresolved', reasons=['native framework error/warning'], abnormal=[])
    if any(counts[key] for key in ['FAIL', 'XPASS']):
        return dict(status='native-failed', reasons=[], abnormal=[])
    if any(counts[key] for key in ['UNRESOLVED', 'ERROR', 'WARNING']) or not native or execution['returncode'] != 0:
        return dict(status='harness-unresolved', reasons=['native framework did not finish cleanly'], abnormal=[])
    if counts['UNSUPPORTED'] and not counts['PASS'] and not counts['XFAIL']:
        return dict(status='native-unsupported', reasons=[], abnormal=[])
    if counts['UNSUPPORTED']:
        return dict(status='native-partial', reasons=['some native assertions unsupported'], abnormal=[])
    return dict(status='native-assertions-passed', reasons=[], abnormal=[])


def binary(command):
    found = shutil.which(command)
    if not found:
        raise ValueError('executable not found: ' + command)
    return str(Path(found).resolve())


def main():
    def interrupted(number, frame):
        # Unwind execute's finally block instead of abandoning native workers.
        raise RuntimeError('native adapter interrupted by signal ' + str(number))
    for number in (signal.SIGHUP, signal.SIGTERM, signal.SIGINT):
        signal.signal(number, interrupted)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--testsuite', type=Path, required=True)
    parser.add_argument('--dejagnu-library', type=Path, required=True)
    parser.add_argument('--runtest', required=True)
    parser.add_argument('--case', type=Path, required=True)
    parser.add_argument('--source-root', type=Path, help='root containing main and companion test sources')
    parser.add_argument('--candidate', required=True)
    parser.add_argument('--host', default='g++')
    parser.add_argument('--mode', choices=['candidate', 'host'], default='candidate')
    parser.add_argument('--dialect', choices=['c++11', 'gnu++11'], default='c++11')
    parser.add_argument('--defaults', choices=['native', 'adapted'], default='native',
                        help='adapted explicitly omits GCC default pedantic-errors/Wno-long-long')
    parser.add_argument('--timeout-seconds', type=float, default=15)
    parser.add_argument('--case-timeout-seconds', type=float, default=90)
    parser.add_argument('--cpu-seconds', type=int, default=30)
    parser.add_argument('--address-space-bytes', type=int, default=1073741824)
    parser.add_argument('--file-bytes', type=int, default=67108864)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    config = vars(args).copy()
    for name in ['timeout_seconds', 'case_timeout_seconds', 'cpu_seconds', 'address_space_bytes', 'file_bytes']:
        if config[name] <= 0:
            parser.error(name + ' must be positive')
    root, source, output = args.testsuite.resolve(), args.case.resolve(), args.output.resolve()
    if not source.is_file() or not (root / 'lib' / 'g++-dg.exp').is_file():
        parser.error('case and native GCC C++ testsuite helpers must exist')
    output.mkdir(parents=True, exist_ok=False)
    (output / 'calls').mkdir()
    (output / 'tmp').mkdir()
    for key in ['candidate', 'host', 'runtest']:
        config[key] = binary(config[key])
    config.update(source_root=str((args.source_root or source.parent).resolve()),
                  log=str(output / 'calls'))
    for key, value in list(config.items()):
        if isinstance(value, Path):
            config[key] = str(value.resolve())
    write_json(output / 'config.json', config)
    extract = output / 'extract.tcl'
    extract.write_text(EXTRACT)
    metadata_result = subprocess.run(['tclsh', str(extract), str(args.dejagnu_library.resolve()), str(source)],
                                     capture_output=True, text=True, timeout=args.timeout_seconds)
    (output / 'metadata.stdout').write_text(metadata_result.stdout)
    (output / 'metadata.stderr').write_text(metadata_result.stderr)
    metadata_result.check_returncode()
    metadata = parse_metadata(metadata_result.stdout)
    write_json(output / 'metadata.json', metadata)
    blockers = []
    if args.mode == 'candidate':
        for directive in metadata:
            if directive['command'] == 'dg-final':
                blockers.append('unavailable dg-final interface (selector applicability unresolved): ' +
                                ' '.join(directive['arguments']))
    suite = output / 'testsuite'
    (suite / 'g++.dg').mkdir(parents=True)
    (suite / 'lib').symlink_to(root / 'lib', target_is_directory=True)
    (suite / 'g++.dg' / 'cppgm.exp').write_text(DRIVER)
    proxy = Path(__file__).with_name('upstream_dg_proxy.py').resolve()
    adapters = output / 'adapter'
    adapters.mkdir()
    for script in [Path(__file__).resolve(), proxy]:
        shutil.copyfile(script, adapters / script.name)
    frozen_proxy = adapters / proxy.name
    wrapper = output / 'compiler-proxy'
    wrapper.write_text('#!/bin/sh\nexec ' + shlex.join([sys.executable, str(frozen_proxy), '--config',
                                                       str(output / 'config.json'), '--']) + ' "$@"\n')
    wrapper.chmod(0o755)
    runtime_wrapper = output / 'runtime-proxy'
    runtime_wrapper.write_text('#!/bin/sh\nexec ' + shlex.join([sys.executable, str(frozen_proxy), '--config',
                                                             str(output / 'config.json'), '--runtime', '--']) + ' "$@"\n')
    runtime_wrapper.chmod(0o755)
    settings = dict(host_triplet='x86_64-pc-linux-gnu', target_triplet='x86_64-pc-linux-gnu',
                    build_triplet='x86_64-pc-linux-gnu', GXX_UNDER_TEST=wrapper,
                    GCC_UNDER_TEST=wrapper, tmpdir=output / 'tmp', CXXFLAGS='-std=' + args.dialect,
                    cppgm_case=source, cppgm_metadata=output / 'native-metadata.tsv',
                    cppgm_runtime_proxy=runtime_wrapper,
                    cppgm_dialect=args.dialect,
                    cppgm_defaults='-pedantic-errors -Wno-long-long' if args.defaults == 'native' else '')
    site = output / 'site.exp'
    site.write_text(''.join('set ' + key + ' ' + tcl(value) + '\n' for key, value in settings.items()) +
                    'set TESTING_IN_BUILD_TREE 0\nset TORTURE_OPTIONS {{-O0}}\n' +
                    'set tool_timeout ' + str(int(args.timeout_seconds + 5)) + '\n' +
                    'set test_timeout ' + str(int(args.timeout_seconds + 5)) + '\n')
    command = [config['runtest'], '--tool', 'g++', '--srcdir', str(suite),
               '--target', 'x86_64-pc-linux-gnu', 'cppgm.exp']
    write_json(output / 'command.json', command)
    manifest = dict(source=dict(path=str(source), sha256=sha256(source)),
                    binaries={key: dict(path=config[key], sha256=sha256(config[key]))
                              for key in ['candidate', 'host', 'runtest']},
                    adapters={str(p): sha256(p) for p in [Path(__file__).resolve(), proxy]},
                    environment={name: os.environ.get(name) for name in
                                 ['PATH', 'LD_LIBRARY_PATH', 'CPATH', 'CPLUS_INCLUDE_PATH',
                                  'LIBRARY_PATH', 'CPPGM_HOST_CXX', 'CPPGM_STDLIB_FLAGS']},
                    native_helpers={str(p.relative_to(root)): sha256(p) for p in (root / 'lib').glob('*.exp')},
                    dejagnu_helpers={str(p.relative_to(args.dejagnu_library)): sha256(p)
                                     for p in args.dejagnu_library.rglob('*.exp') if p.is_file()},
                    defaults=args.defaults, dialect=args.dialect,
                    note='Single explicit variant; original native directives. GCC_UNDER_TEST also uses proxy. '
                         'No original multi-standard driver or torture matrix pass is claimed.')
    write_json(output / 'manifest.json', manifest)
    previous = Path.cwd()
    environment = {name: os.environ.get(name) for name in ['DEJAGNU', 'TMPDIR', 'CPPGM_DG_PHASE', 'LC_ALL', 'LANG']}
    try:
        os.chdir(output)
        os.environ.update(DEJAGNU=str(site), TMPDIR=str(output / 'tmp'), CPPGM_DG_PHASE='initialization', LC_ALL='C', LANG='C')
        execution_config = dict(config, timeout_seconds=args.case_timeout_seconds,
                                compiler_error_scan=False,
                                cpu_seconds=max(args.cpu_seconds, int(args.case_timeout_seconds)))
        execution, stdout, stderr = execute(command, output / 'runtest', execution_config)
    finally:
        os.chdir(previous)
        for key, value in environment.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value
    calls = [dict(json.loads(path.read_text()), record=str(path)) for path in sorted((output / 'calls').glob('*.json'))]
    summary = output / 'g++.sum'
    native = assertions(summary.read_text() if summary.is_file() else '')
    result = disposition(native, calls, execution, blockers)
    result.update(assertions=native, native_counts=dict(Counter(item['status'] for item in native)),
                  execution=execution, calls=calls,
                  coverage='adapted native oracle; not full upstream pass',
                  full_upstream_pass=False)
    write_json(output / 'result.json', result)
    print(json.dumps({key: result[key] for key in ['status', 'native_counts', 'reasons', 'coverage']}))
    return 0 if result['status'] in {'native-assertions-passed', 'native-unsupported'} else 1


if __name__ == '__main__':
    sys.exit(main())

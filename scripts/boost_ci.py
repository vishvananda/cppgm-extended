#!/usr/bin/env python3
"""Run the bounded Boost 1.92 advisory selection and review actual B2 results."""
import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import time
import xml.etree.ElementTree as ET


SUITES = {
    'libs/accumulators/test': 49,
    'libs/container/test': 89,
    'libs/function/test': 49,
    'libs/fusion/test': 221,
    'libs/json/test': 50,
    'libs/mp11/test': 234,
    'libs/mpl/test': 99,
    'libs/optional/test': 74,
    'libs/proto/test': 23,
    'libs/type_erasure/test': 60,
    'libs/type_traits/test': 218,
    'libs/uuid/test': 80,
    'libs/variant2/test': 84,
    'libs/xpressive/test': 51,
}
ARCHIVE_SHA256 = '5c1d40cb8e19adbf740a4ec2da35b3e58f3f5804b1dce44deb53df72193cbc6c'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def review(xml_path, log_path, calls_path, project, expected):
    tree = ET.parse(xml_path).getroot()
    boost = Path(tree.findtext('directory')).resolve()
    declarations = []
    for test in tree.findall('test'):
        target = test.findtext('target')
        if target.startswith(str(boost) + '/'):
            target = target[len(str(boost)) + 1:]
        if target.split('//')[0] == project:
            declarations.append((test, target))
    require(len(declarations) == expected, f'{project}: expected {expected} declarations, got {len(declarations)}')
    require(not re.search(r'^\.\.\.failed updating \d+ targets?\.\.\.$', log_path.read_text(), re.M), 'B2 reported failed targets')
    calls = [json.loads(line) for line in calls_path.read_text().splitlines()]
    require(calls and all(c['returncode'] in (0, 1) for c in calls), 'compiler crash or invalid compiler status')
    by_directory = {}
    for call in calls:
        arguments = call['args']
        if '-c' not in arguments or '-o' not in arguments:
            continue
        product = Path(arguments[arguments.index('-o') + 1])
        if not product.is_absolute():
            product = Path(call['cwd']) / product
        for parent in product.resolve().parents:
            by_directory.setdefault(parent, []).append(call)
    counts = Counter()
    rows = []
    actions = tree.findall('action')
    for test, target in declarations:
        kind = test.attrib['type']
        require(kind in ('COMPILE', 'COMPILE_FAIL', 'LINK', 'LINK_FAIL', 'RUN', 'RUN_FAIL'), f'unreviewed test kind {kind}')
        directory, name = target.split('//')
        expectation = 'testing%testing.expect-' + ('failure' if kind.endswith('_FAIL') else 'success')
        matching = [a for a in actions if a.findtext('name') == expectation and
                    '/boost/bin.v2/' + directory + '/' + name + '/' in (a.findtext('path') or '') and
                    Path(a.findtext('path')).name == name]
        require(len(matching) == 1, f'missing or duplicate expectation for {target}')
        action = matching[0]
        marker = Path(action.findtext('path'))
        require(action.attrib['status'] == '0' and marker.read_text().strip() == 'passed', f'failed {target}')
        compiled = by_directory.get(marker.parent.resolve(), [])
        require(compiled and all(c['route'] == 'cppgm' for c in compiled), f'no cppgm compilation for {target}')
        require(any(c['returncode'] == 1 for c in compiled) if kind == 'COMPILE_FAIL' else
                all(c['returncode'] == 0 for c in compiled), f'incorrect compilation result for {target}')
        row = dict(target=target, kind=kind, marker_sha256=digest(marker), compiler_invocations=len(compiled))
        if kind in ('RUN', 'RUN_FAIL'):
            output = marker.with_suffix('.output')
            status = re.search(r'EXIT STATUS:\s*(\d+)\s*$', output.read_text(errors='replace'))
            require(status is not None, f'no native exit status for {target}')
            code = int(status[1])
            require((code == 0) == (kind == 'RUN') and code < 128, f'incorrect native result for {target}')
            capture = [a for a in actions if a.findtext('name') == 'testing%testing.capture-output' and
                       a.findtext('path') == str(marker.with_suffix('.run'))]
            require(len(capture) == 1 and (capture[0].attrib['status'] == '0') == (kind == 'RUN'), f'no actual execution for {target}')
            if project == 'libs/json/test' and name == '_gdb-printers.test':
                require('boost-ci-gdb' in capture[0].findtext('command', ''), 'JSON printer test did not execute GDB')
            row.update(native_status=code, output_sha256=digest(output))
        rows.append(row)
        counts[kind] += 1
    return dict(project=project, declarations=len(declarations), passed=len(rows), counts=dict(counts), tests=rows,
                xml_sha256=digest(xml_path), invocations_sha256=digest(calls_path))


def setup(boost, output):
    config = output / 'config'
    config.mkdir()
    # Select Boost's real compiler config while retaining its platform/stdlib and
    # feature detection. This matches the campaign; no capabilities are forced.
    (config / 'compiler.hpp').write_text('#if defined(__clang__)\n#include <boost/config/compiler/clang.hpp>\n#else\n#include <boost/config/compiler/gcc.hpp>\n#endif\n')
    (config / 'user.hpp').write_text('#define BOOST_COMPILER_CONFIG "' + str(config / 'compiler.hpp') + '"\n')
    adapter = Path(__file__).with_name('boost_ci_toolchain.py').resolve()
    gdb = output / 'boost-ci-gdb'
    safe = output / 'build/boost/bin.v2/libs/json/test'
    gdb.write_text('#!/usr/bin/env python3\nimport os,sys\nos.execv("/usr/bin/gdb", ["gdb", "-iex", ' +
                   repr('add-auto-load-safe-path ' + str(safe)) + ', *sys.argv[1:]])\n')
    gdb.chmod(0o755)
    jam = config / 'user-config.jam'
    jam.write_text('using gcc : cppgm : "' + str(adapter) + '" : <archiver>/usr/bin/ar <cxxflags>-include <cxxflags>"' + str(config / 'user.hpp') + '" ;\n'
                   'using python : : /usr/bin/python3 ;\nimport-search "' + str(boost / 'libs/json/pretty_printers') + '" ;\n'
                   'using boost-pretty-printers : "' + str(gdb) + '" ;\n')
    return jam


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--boost-root', type=Path, required=True)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--compiler', type=Path, default=Path('dev/cppgm++'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--jobs', type=int, default=4)
    args = parser.parse_args()
    start = time.monotonic()
    require(digest(args.archive) == ARCHIVE_SHA256, 'Boost archive checksum mismatch')
    boost, output, compiler = args.boost_root.resolve(), args.output.resolve(), args.compiler.resolve()
    require(not output.exists(), 'use a fresh output directory')
    output.mkdir(parents=True)
    host_cxx, host_cc = shutil.which('g++'), shutil.which('gcc')
    require(host_cxx and host_cc and compiler.is_file(), 'missing compiler')
    jam = setup(boost, output)
    env = dict(os.environ, BOOST_CI_COMPILER=str(compiler), BOOST_CI_HOST_CXX=host_cxx,
               BOOST_CI_HOST_CC=host_cc, CPPGM_HOST_CXX=host_cxx, CPPGM_STDLIB_FLAGS='')
    report = dict(boost='1.92.0', archive_sha256=ARCHIVE_SHA256, compiler_sha256=digest(compiler), suites=[], status='running')

    def save():
        report['seconds'] = time.monotonic() - start
        temporary = output / 'results.json.tmp'
        temporary.write_text(json.dumps(report, indent=2) + '\n')
        temporary.replace(output / 'results.json')

    save()
    try:
        if not (boost / 'b2').exists():
            with (output / 'bootstrap.log').open('w') as log:
                subprocess.run(['./bootstrap.sh', '--with-toolset=gcc'], cwd=boost, env=env, stdout=log, stderr=subprocess.STDOUT, check=True)
        for project, expected in SUITES.items():
            report['current_suite'] = project
            save()
            case = output / project.replace('/', '-')
            case.mkdir()
            env['BOOST_CI_INVOCATIONS'] = str(case / 'invocations.jsonl')
            command = [str(boost / 'b2'), '--ignore-site-config', '--user-config=' + str(jam), '-j' + str(args.jobs),
                       'toolset=gcc-cppgm', 'cxxstd=11', 'variant=debug', 'link=static', 'runtime-link=shared',
                       'threading=multi', '--build-dir=' + str(output / 'build'), '--reconfigure', '-a', '-d2',
                       project, '--out-xml=' + str(case / 'b2.xml'), '--dump-tests']
            # The original JSON Jamfile declares this target explicit; the
            # project alone builds its 49 ordinary tests but omits GDB.
            if project == 'libs/json/test':
                command.append('libs/json/test//gdb-printers')
            suite_start = time.monotonic()
            with (case / 'b2.log').open('w') as log:
                log.write(shlex.join(command) + '\n'); log.flush()
                process = subprocess.run(command, cwd=boost, env=env, stdout=log, stderr=subprocess.STDOUT)
            require(process.returncode == 0, f'B2 exited {process.returncode}: {project}')
            result = review(case / 'b2.xml', case / 'b2.log', case / 'invocations.jsonl', project, expected)
            result['seconds'] = time.monotonic() - suite_start
            report['suites'].append(result)
            save()
            print(f"{project}: {result['passed']} passed in {result['seconds']:.1f}s", flush=True)
        report['status'] = 'passed'
        report.pop('current_suite', None)
    except Exception as error:
        report.update(status='failed', error=str(error))
        log = output / report.get('current_suite', '').replace('/', '-') / 'b2.log'
        if log.is_file():
            print('Failure log: ' + str(log), file=sys.stderr)
            lines = log.read_text(errors='replace').splitlines()
            for index, line in enumerate(lines):
                if re.search(r'ERROR:|error:|Traceback|failed testing|failed gcc|failed updating', line):
                    print('\n'.join(lines[max(0, index - 2):index + 8]), file=sys.stderr)
        raise
    finally:
        save()


if __name__ == '__main__':
    main()

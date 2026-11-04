#!/usr/bin/env python3
"""Check the LowIR comparison seams on one assignment lane.

usage: check_lowir_seams.py <assignment-dir> <lane> [--modes A,B] [--results FILE] [--jobs N]

For every rewrite mode of lowir_seam_rewrite.py: copy the lane, apply the
rewrite to each `.my` output, compare the copies with their references
through the assignment's own harness (KEEP_GOING=1 scripts/compare_results.pl
ref my), and hold the lane to one invariant:

  - a presentation or normalization rewrite passes every fixture;
  - a convention rewrite fails at least one fixture it changed;
  - the sentence each mode points at is still in pa8/lowir.md.

So everything the comparison rejects is a written convention, and
everything it absorbs is a written normalization.  A new rejection with no
sentence behind it fails the lane.  Exit status 0 when the invariant holds
for every mode; the table it prints is also the record `--results` writes.
"""
import os
import concurrent.futures
import hashlib
import itertools
import re
import shutil
import subprocess
import sys
import tempfile

SCRIPTS = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(SCRIPTS)
sys.path.insert(0, SCRIPTS)
from lowir_seam_rewrite import MODES, rewrite  # noqa: E402

CONTRACT = os.path.join(REPO, 'pa8', 'lowir.md')


def parse_arguments(argv):
    positional = []
    modes = None
    results = None
    jobs = int(os.environ.get('CPPGM_TEST_JOBS', '1'))
    i = 1
    while i < len(argv):
        if argv[i] == '--modes':
            modes = argv[i + 1].split(',')
            i += 2
        elif argv[i] == '--results':
            results = argv[i + 1]
            i += 2
        elif argv[i] == '--jobs':
            jobs = int(argv[i + 1])
            i += 2
        else:
            positional.append(argv[i])
            i += 1
    if len(positional) != 2 or jobs < 1:
        sys.exit(__doc__)
    return positional[0], positional[1], modes, results, jobs


def run_result_name(name):
    return subprocess.check_output(
        ['perl', '-I' + SCRIPTS, '-MCppgmBatchWorker=run_result_name',
         '-e', 'print run_result_name($ARGV[0])', name], text=True,
    )


def copy_lane(source, target, suffix):
    # A worktree can retain many result namespaces. Only this invocation's
    # outputs participate; copying all older runs for every rewrite exhausts
    # temporary-file quotas without adding coverage.
    endings = tuple(suffix + extension for extension in ('', '.exit_status', '.stdout', '.stderr'))
    def ignore(_directory, names):
        return [name for name in names
                if re.search(r'\.(?:my|check)(?:\.|$)', name) and not name.endswith(endings)]
    shutil.copytree(source, target, symlinks=False, ignore=ignore)


def run_mode(assignment, lane, mode, scratch, source_suffix):
    """(passed, total, changed, first failure line) for one rewrite of the lane."""
    copy = os.path.join(scratch, mode)
    copy_lane(os.path.join(assignment, lane), copy, source_suffix)
    # Convention rewrites deliberately fail. Give each comparison its own
    # namespace so parallel modes cannot remove another mode's failure marker
    # or the report's real assignment failure marker.
    run_id = 'seam-' + hashlib.sha256(os.fsencode(copy)).hexdigest()[:24]
    suffix = '.my.' + run_id
    for name in os.listdir(copy):
        for extension in ('', '.exit_status', '.stdout', '.stderr'):
            ending = source_suffix + extension
            if name.endswith(ending):
                os.rename(os.path.join(copy, name),
                          os.path.join(copy, name[:-len(ending)] + suffix + extension))
                break
    changed = 0
    for name in sorted(os.listdir(copy)):
        if not name.endswith(suffix):
            continue
        path = os.path.join(copy, name)
        with open(path) as f:
            before = f.read()
        after = rewrite(mode, before)
        if after == before:
            continue
        with open(path, 'w') as f:
            f.write(after)
        # Only an output the harness compares with a reference counts as
        # changed: a fixture judged by an `x.ref.expect` sidecar, or whose
        # reference run did not succeed, never reaches the comparison.
        base = path[:-len(suffix)]
        if not os.path.exists(base + '.ref') or os.path.exists(base + '.ref.expect'):
            continue
        status = base + '.ref.exit_status'
        if os.path.exists(status):
            with open(status) as f:
                if f.read().strip() not in ('EXIT_SUCCESS', '0'):
                    continue
        changed += 1
    counts = os.path.join(scratch, mode + '.counts')
    environment = dict(os.environ, KEEP_GOING='1', CPPGM_TEST_COUNTS_FILE=counts,
                       CPPGM_TEST_RUN_ID=run_id, CPPGM_TEST_JOBS='1')
    # The lane is about the relaxed comparison by definition; CI's byte-exact
    # switch (CPPGM_LOWIR_DIRECT_TEXT_COMPARE) does not apply to it.
    environment.pop('CPPGM_LOWIR_DIRECT_TEXT_COMPARE', None)
    marker = os.path.join(assignment, '.test_failed.' + run_id)
    try:
        run = subprocess.run(['scripts/compare_results.pl', 'ref', 'my', copy],
                             cwd=assignment, env=environment, text=True,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    finally:
        if os.path.exists(marker):
            os.remove(marker)
    passed = total = 0
    try:
        with open(counts) as f:
            for line in f:
                p, t = line.split()
                passed += int(p)
                total += int(t)
        os.remove(counts)
    except (OSError, ValueError):
        pass
    failure = ''
    for line in run.stdout.splitlines():
        if 'ERROR' in line or 'FAIL' in line:
            failure = re.sub(r'^[^:]*: ', '', line)[:120]
            break
    if total == 0 and not failure:
        failure = f'no comparison ran (exit {run.returncode})'
    return passed, total, changed, failure


def main(argv):
    assignment, lane, only, results_path, jobs = parse_arguments(argv)
    assignment = os.path.abspath(assignment)
    with open(CONTRACT) as f:
        contract = ' '.join(f.read().split()).lower()
    rows = []
    problems = []
    scratch = tempfile.mkdtemp(prefix='lowir-seams-')
    try:
        selected = [entry for entry in MODES if not only or entry[0] in only]
        source_suffix = '.' + run_result_name('my')
        if jobs > 1:
            with concurrent.futures.ProcessPoolExecutor(max_workers=jobs) as executor:
                outcomes = list(executor.map(
                    run_mode, itertools.repeat(assignment), itertools.repeat(lane),
                    [entry[0] for entry in selected], itertools.repeat(scratch),
                    itertools.repeat(source_suffix)))
        else:
            outcomes = [run_mode(assignment, lane, entry[0], scratch, source_suffix)
                        for entry in selected]
        for (mode, kind, rule), (passed, total, changed, failure) in zip(selected, outcomes):
            rows.append((mode, kind, passed, total, changed, failure))
            if ' '.join(rule.split()).lower() not in contract:
                problems.append(f'{mode}: the rule "{rule}" is not stated in pa8/lowir.md')
            if total == 0:
                problems.append(f'{mode}: {failure}')
            elif kind in ('presentation', 'normalization'):
                if passed != total:
                    problems.append(f'{mode}: a {kind} rewrite was rejected on {total - passed} fixture(s): {failure}')
            elif changed and passed == total:
                problems.append(f'{mode}: a convention rewrite changed {changed} output(s) and every one was accepted;'
                                f' either the comparison stopped enforcing the convention or it must become a written normalization')
    finally:
        shutil.rmtree(scratch, ignore_errors=True)

    lane_name = os.path.join(os.path.basename(assignment), lane)
    header = f'{"mode":<22} {"kind":<14} {"pass/total":<11} {"changed":<8} first failure'
    lines = [f'# LowIR comparison seams on {lane_name}', header]
    for mode, kind, passed, total, changed, failure in rows:
        lines.append(f'{mode:<22} {kind:<14} {f"{passed}/{total}":<11} {changed:<8} {failure}')
    report = '\n'.join(lines)
    print(report)
    if results_path:
        with open(results_path, 'w') as f:
            f.write(report + '\n')
    if problems:
        print(f'{lane_name} seams: FAIL')
        for problem in problems:
            print('  ' + problem)
        return 1
    print(f'{lane_name} seams: PASS ({len(rows)} rewrites)')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))

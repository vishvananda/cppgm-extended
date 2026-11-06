"""Project upstream test receipts onto C++ language outcomes.

Diagnostic spelling, quantity, notes and recovery are not candidate oracles.
The upstream host resolves active expected-error directives; the candidate
only needs a normal diagnostic for a negative case. Positive cases retain
their compilation, linking, execution and program-output obligations.

This is an outcome screen. A mismatch needs standards/root-cause triage before
it is called a compiler defect. Explicit scope dispositions remain separate.
"""
from collections import Counter
from pathlib import Path


def primary_calls(data, source):
    """Select actual test translations, excluding native capability probes."""
    source = str(Path(source).resolve())
    found = []
    for call in data.get('calls', []):
        if call.get('role') != 'test':
            continue
        directory = Path(call.get('cwd', '.'))
        for item in call.get('sources', []):
            if str(Path(item['path']).resolve()) == source:
                found.append(call)
                break
        else:
            for argument in call.get('incoming', []):
                if argument.startswith('-'):
                    continue
                path = Path(argument)
                if str((directory / path).resolve()) == source:
                    found.append(call)
                    break
    return found


def translation_steps(call, source):
    """Keep compiler stages consuming source; exclude subsequent host linking."""
    source = str(Path(source).resolve())
    directory = Path(call.get('cwd', '.'))
    return [step for step in call.get('steps', [])
            if any(not arg.startswith('-') and
                   str((directory / arg).resolve()) == source
                   for arg in step.get('command', [])[1:])]


def has_diagnostic(call, source):
    # Compiler output alone cannot establish a specific language violation.
    # This provisional screen checks presence; reduced regressions establish
    # which rule is violated. Never count a separate linker diagnostic.
    for step in translation_steps(call, source):
        for stream in ['stderr', 'stdout']:
            path = step.get(stream + '_path')
            value = Path(path).read_text(errors='replace') if path else step.get(stream, '')
            if value.strip():
                return True
    return False


def compile_only(call):
    return any(arg in {'-c', '-S', '-fsyntax-only'} for arg in call.get('incoming', []))


def policy_flags(call):
    """Do not infer language validity from warning-to-error policy."""
    return [arg for arg in call.get('incoming', [])
            if arg == '-pedantic-errors' or arg.startswith(('-Werror', '-Wno-error'))]


def runtime_assertions(data):
    return [item for item in data.get('assertions', [])
            if 'execution test' in item['text'] or 'output pattern' in item['text']]


def unprojected_assertions(data):
    known = ['test for errors', 'test for warnings', 'test for excess errors',
             'test for bogus messages', 'test for messages', 'execution test',
             'output pattern']
    return [item for item in data.get('assertions', [])
            if not any(text in item['text'] for text in known)]


def classify(host, candidate, source, disposition=None):
    """Compare one language variant without diagnostic parity requirements."""
    result = dict(status=None, reason=None, expected=None,
                  diagnostic_parity_required=False)

    def finish(status, reason):
        result.update(status=status, reason=reason)
        return result

    if disposition:
        result['disposition'] = disposition
        return finish(disposition['status'], disposition['reason'])
    host_calls = primary_calls(host, source)
    candidate_calls = primary_calls(candidate, source)
    if len(host_calls) != 1:
        return finish('baseline-unresolved', 'Expected one actual host translation; capability probes are not test execution.')
    h = host_calls[0]
    if '-E' in h.get('incoming', []) or unprojected_assertions(host):
        return finish('baseline-unresolved', 'Preprocessor output or another native oracle still needs a reviewed language projection.')
    if policy_flags(h):
        result['policy_flags'] = policy_flags(h)
        return finish('diagnostic-policy-review', 'Warning escalation must be separated from language validity before replay.')
    if h.get('classification') not in {'success', 'rejection'}:
        return finish('baseline-unresolved', 'The actual host translation did not complete normally.')
    negative = any('test for errors' in item['text'] for item in host.get('assertions', []))
    result['expected'] = 'diagnose-invalid' if negative else 'accept-valid'
    if negative and not compile_only(h):
        return finish('baseline-unresolved', 'A negative combined compile/link command needs a separate translation oracle.')
    if negative and not has_diagnostic(h, source):
        return finish('baseline-unresolved', 'An active negative expectation has no actual host diagnostic.')
    if not negative and h.get('classification') != 'success':
        return finish('baseline-unresolved', 'Host rejected a variant without an active negative expectation.')
    if len(candidate_calls) != 1:
        result['candidate_native_status'] = candidate.get('status')
        return finish('harness-blocked', 'No single actual candidate translation is available; retain the interface/probe blocker.')
    c = candidate_calls[0]
    result['candidate_translation'] = c.get('classification')
    result['candidate_diagnosed'] = has_diagnostic(c, source)
    if policy_flags(c):
        result['policy_flags'] = policy_flags(c)
        return finish('diagnostic-policy-review', 'Candidate warning-escalation flags need a language-only replay.')
    if c.get('classification') == 'blocked' or c.get('classification', '').startswith('harness-'):
        return finish('harness-blocked', 'Candidate translation was blocked by an interface or capability check.')
    if c.get('classification') not in {'success', 'rejection'}:
        return finish('compiler-abnormal', 'A crash, timeout or internal/resource failure is not a language rejection.')
    steps = translation_steps(c, source)
    if not steps:
        return finish('harness-blocked', 'No recorded candidate compiler stage consumes the actual test source.')
    if any(step.get('classification') not in {'success', 'rejection'} for step in steps):
        return finish('compiler-abnormal', 'An abnormal compiler stage cannot satisfy a language check.')
    if negative:
        if result['candidate_diagnosed']:
            # A conforming implementation can diagnose an ill-formed input
            # and then accept it as an extension. Its runtime is not an oracle
            # for a well-defined standard program.
            return finish('outcome-match', 'The negative translation received a diagnostic; wording, count and recovery are irrelevant.')
        return finish('outcome-mismatch', 'The negative translation received no diagnostic.')
    if c['classification'] != 'success':
        return finish('outcome-mismatch', 'Candidate rejected a positive translation.')
    if unprojected_assertions(candidate) or candidate.get('status') in {'native-partial', 'native-unsupported'}:
        return finish('harness-blocked', 'An additional native obligation has not been projected onto a language-level check.')
    h_runtime = runtime_assertions(host)
    c_runtime = runtime_assertions(candidate)
    h_runs = [call for call in host.get('calls', []) if call.get('role') == 'runtime']
    c_runs = [call for call in candidate.get('calls', []) if call.get('role') == 'runtime']
    if h_runtime or h_runs:
        result['expected'] = 'accept-link-run'
        if not h_runtime or not h_runs or any(x['status'] != 'PASS' for x in h_runtime):
            return finish('baseline-unresolved', 'The host runtime/output oracle is incomplete or not passing.')
        if any(x.get('classification') != 'success' for x in h_runs):
            return finish('baseline-unresolved', 'Host termination requires explicit semantic adjudication.')
        def obligations(items):
            return Counter('output' if 'output pattern' in item['text'] else 'execution'
                           for item in items)

        if len(c_runs) != len(h_runs) or obligations(c_runtime) != obligations(h_runtime):
            return finish('harness-blocked', 'Candidate execution or an expected program-output check is missing.')
        if any(x.get('classification') != 'success' for x in c_runs):
            return finish('runtime-mismatch', 'Candidate runtime did not complete as required by the host oracle.')
        if any(x['status'] != 'PASS' for x in c_runtime):
            return finish('runtime-mismatch', 'Candidate execution or program output violates the upstream runtime oracle.')
    if candidate.get('status', '').startswith('harness-'):
        # A final assembly/IR check can be blocked after compilation. Do not
        # silently turn partial evidence into a complete language-test pass.
        return finish('harness-blocked', 'A retained final/interface blocker still needs a reviewed language-level projection.')
    return finish('outcome-match', 'Positive translation and every declared link/runtime/output obligation completed.')

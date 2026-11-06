"""Explicit Clang RUN-to-language projections, without diagnostic parity.

The original RUN and every unprojected output/runtime obligation remain in the
receipt. A projected syntax/object result is never a full upstream test pass.
"""
from pathlib import Path
import re
import struct


class ProjectionBlocked(ValueError):
    pass


DIALECTS = {'c++11', 'gnu++11', 'c++0x', 'gnu++0x'}
LINUX_X86_64 = {'x86_64-linux-gnu', 'x86_64-pc-linux-gnu', 'x86_64-unknown-linux-gnu'}
OUTPUT_FLAGS = {'-emit-llvm', '-emit-llvm-only', '-S', '-ast-print', '-ast-dump'}
RETAINED_FLAGS = {'-fexceptions', '-fcxx-exceptions', '-O0', '-O1', '-O2', '-O3'}


def project_arguments(arguments, source, dialect, target, resource):
    """Map only reviewed cc1 policies; unsupported driver options never reject C++."""
    if dialect not in {'c++11', 'gnu++11'} or target not in LINUX_X86_64:
        raise ProjectionBlocked('Outside declared Linux x86-64 C++11 configuration')
    if not arguments or arguments[0] != '%clang_cc1':
        raise ProjectionBlocked('Compiler position/mode is not a supported %clang_cc1 invocation')
    source = str(Path(source).resolve())
    native, candidate, verification, adaptations, interfaces = [], [], [], [], []
    inputs, explicit_standards = [], []
    exceptions = False  # Native cc1 enables C++ exceptions only by explicit option.
    i = 1
    while i < len(arguments):
        argument = arguments[i]
        i += 1
        if not isinstance(argument, str):
            raise ProjectionBlocked('Glob or nonliteral compiler argument')
        if re.search(r'%[A-Za-z/{]', argument):
            raise ProjectionBlocked('Unresolved native substitution: ' + argument)
        if argument.startswith('-std='):
            explicit_standards.append(argument[5:])
            if argument[5:] not in DIALECTS:
                raise ProjectionBlocked('RUN explicitly selects a different language standard: ' + argument)
        elif argument == '-verify' or argument.startswith('-verify='):
            verification.append(argument)
        elif argument.startswith('-verify-ignore-unexpected'):
            verification.append(argument)
        elif argument == '-fsyntax-only':
            adaptations.append('syntax-only action strengthened to relocatable object generation')
        elif argument in OUTPUT_FLAGS or argument.startswith(('-ast-dump=', '-ast-print=')):
            interfaces.append(argument)
        elif argument in {'-E', '-emit-pch', '-include-pch'}:
            raise ProjectionBlocked('Preprocessor/PCH obligation needs its own reviewed projection: ' + argument)
        elif argument == '-o':
            if i == len(arguments):
                raise ProjectionBlocked('Missing native output argument')
            adaptations.append('native output destination retained as metadata: ' + arguments[i])
            i += 1
        elif argument in {'-triple', '-target', '--target'}:
            if i == len(arguments) or arguments[i] not in LINUX_X86_64:
                raise ProjectionBlocked('Unsupported/missing native target')
            target = arguments[i]
            i += 1
        elif argument.startswith(('-triple=', '--target=')):
            if argument.split('=', 1)[1] not in LINUX_X86_64:
                raise ProjectionBlocked('Native target is outside declared Linux x86-64 scope')
            target = argument.split('=', 1)[1]
        elif argument in {'-I', '-isystem', '-include', '-D', '-U'}:
            if i == len(arguments):
                raise ProjectionBlocked('Missing preprocessor operand: ' + argument)
            native.extend([argument, arguments[i]])
            candidate.extend([argument, arguments[i]])
            i += 1
        elif argument.startswith(('-I', '-D', '-U')):
            native.append(argument)
            candidate.append(argument)
        elif argument in {'-pedantic-errors', '-Wno-narrowing'} or argument.startswith('-Werror'):
            raise ProjectionBlocked('Diagnostic/language policy needs explicit adjudication: ' + argument)
        elif argument.startswith('-W') or argument in {'-w', '-pedantic'}:
            native.append(argument)
            adaptations.append('optional diagnostic policy omitted from candidate: ' + argument)
        elif argument in {'-fno-exceptions', '-fno-rtti'}:
            raise ProjectionBlocked('Candidate policy capability is unproven/unsupported: ' + argument)
        elif argument == '-fexperimental-new-constant-interpreter':
            native.append(argument)
            adaptations.append('native interpreter implementation selector has no candidate equivalent')
        elif argument in RETAINED_FLAGS:
            native.append(argument)
            if argument == '-fcxx-exceptions':
                exceptions = True
            elif argument != '-fexceptions':
                candidate.append(argument)
        elif argument.startswith('-'):
            raise ProjectionBlocked('Unreviewed native semantic/backend option: ' + argument)
        elif str(Path(argument).resolve()) == source:
            inputs.append(argument)
        else:
            raise ProjectionBlocked('Unknown source/input/substitution: ' + argument)
    if len(inputs) != 1:
        raise ProjectionBlocked('Exactly one known translation-unit source is required')
    if len(set(explicit_standards)) > 1 or len(verification) > 1:
        raise ProjectionBlocked('Conflicting standards or verifier controls need explicit review')
    candidate.append('-fexceptions' if exceptions else '-fno-exceptions')
    if explicit_standards != [dialect]:
        adaptations.append('declared dialect projection: ' + str(explicit_standards or ['cc1 default']) + ' -> ' + dialect)
    common = ['-std=' + dialect]
    native = [*common, '-triple', target, '-internal-isystem', str(resource), '-nostdsysteminc', *native]
    candidate = [*common, '--target=' + target, '-nostdinc', '-isystem', str(resource), *candidate]
    return dict(native_args=native, candidate_args=candidate, verification=verification,
                unprojected_interfaces=interfaces, adaptations=adaptations,
                native_cxx_exceptions=exceptions, native_rtti=True,
                coverage='projected language/object obligation; no full upstream RUN pass')


def elf_object(path):
    data = Path(path).read_bytes()
    if len(data) < 64 or data[:7] != b'\x7fELF\x02\x01\x01':
        return False
    kind, machine, version = struct.unpack_from('<HHI', data, 16)
    section_offset = struct.unpack_from('<Q', data, 40)[0]
    header_size, _, _, section_size, count, names = struct.unpack_from('<HHHHHH', data, 52)
    if (kind, machine, version, header_size, section_size) != (1, 62, 1, 64, 64):
        return False
    if section_offset < 64 or section_offset + 64 > len(data):
        return False
    if count == 0:
        count = struct.unpack_from('<Q', data, section_offset + 32)[0]
    if names == 0xffff:
        names = struct.unpack_from('<I', data, section_offset + 40)[0]
    if not count or names >= count or section_offset + section_size * count > len(data):
        return False
    for index in range(count):
        section = section_offset + section_size * index
        section_kind = struct.unpack_from('<I', data, section + 4)[0]
        offset, size = struct.unpack_from('<QQ', data, section + 24)
        if section_kind != 8 and offset + size > len(data):  # SHT_NOBITS owns no file bytes.
            return False
    return True


def diagnose(receipt):
    """Only compiler stderr is a diagnostic source; generated IR/stdout is not."""
    return bool(Path(receipt['stderr_path']).read_text(errors='replace').strip())


def assess(discovery, host, candidate, projection, adjudication=None):
    """Assess the projected obligation while keeping missing coverage explicit."""
    result = dict(status=None, reason=None, expected=None,
                  full_upstream_pass=False, diagnostic_parity_required=False,
                  unprojected_interfaces=projection['unprojected_interfaces'])

    def finish(status, reason):
        result.update(status=status, reason=reason)
        return result

    if discovery.get('status') != 'expectations-discovered':
        return finish('harness-blocked', discovery.get('reason', 'Native expectation discovery incomplete'))
    if discovery.get('same_preprocessed_tokens') is not True:
        return finish('preprocessing-context-review', 'Candidate and native host do not see the same tokens/branches')
    if discovery.get('active_pragma'):
        return finish('configuration-blocked', 'An active pragma requires reviewed semantic-policy equivalence')
    policy = set(discovery.get('language_policy_tokens', []))
    if not discovery.get('native_cxx_exceptions', False) and policy & {'throw', 'try', 'catch'}:
        return finish('configuration-blocked', 'Disabled-exception native syntax cannot establish a standard C++ language negative')
    if not discovery.get('native_rtti', True) and policy & {'typeid', 'dynamic_cast'}:
        return finish('configuration-blocked', 'Disabled-RTTI native syntax needs proven candidate policy equivalence')
    if host['classification'] not in {'success', 'rejection'}:
        return finish('baseline-unresolved', 'Native host did not complete compilation normally')
    negative = discovery['expected_error']
    result['expected'] = 'diagnose-invalid' if negative else 'accept-positive'
    if adjudication:
        result['adjudication'] = adjudication
        if adjudication['expected'] == 'not-required':
            return finish('not-required', adjudication['reason'])
        if adjudication['expected'] != result['expected']:
            return finish('standards-review', 'Reviewed language rule differs from active upstream expectation')
    if negative and not diagnose(host):
        return finish('baseline-unresolved', 'Negative host translation has no compilation diagnostic')
    if not negative and host['classification'] != 'success':
        return finish('baseline-unresolved', 'Host rejected the positive projection')
    if candidate['classification'] not in {'success', 'rejection'}:
        return finish('compiler-abnormal', 'A crash, timeout or internal/resource failure is not a language rejection')
    if negative:
        result['candidate_diagnosed'] = diagnose(candidate)
        if adjudication is None:
            return finish('negative-needs-adjudication', 'Diagnostic presence is recorded, but the required language rule has not been adjudicated')
        return finish('language-obligation-match' if result['candidate_diagnosed'] else 'language-obligation-mismatch',
                      'Required diagnostic present' if result['candidate_diagnosed'] else 'Required diagnostic missing')
    if candidate['classification'] != 'success':
        return finish('language-obligation-mismatch', 'Candidate rejected a positive object-generation projection')
    return finish('language-obligation-match', 'Positive object generation completed; remaining interfaces are retained separately')

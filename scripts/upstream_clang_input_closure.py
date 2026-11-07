"""Audited local inputs for projected Clang language checks, without executing cfgs.

This deliberately excludes arbitrary conditionals, system headers, generated
inputs and macro-built include names. Header guards are the only conditional
form mapped here. Every consumed file must be tracked in the pinned corpus.
"""
import hashlib
from pathlib import Path
import re


# These native configs only change discovery suffixes or substitutions unused by
# the accepted literal %clang_cc1 commands. Unknown configs remain blockers.
CONFIGS = {
    'clang/test/Sema/lit.local.cfg':
        ('6ab1fc174d02aba387eb616260d83f177639fb294863d4988ae0f366f89cf609',
         'Only rejects %clang driver substitution; projected RUN uses literal %clang_cc1'),
    'clang/test/Frontend/lit.local.cfg':
        ('a83a1ce02b190580ff529189bd70c22208c73d7aa2ec5d974b0ebad34609ad4c',
         'Discovery suffix list includes this C++ source; no compilation policy change'),
    'clang/test/CodeGenHIP/lit.local.cfg':
        ('f08b3d7a96a0316c36e762e7a848e4c72948d1b607c6c1d4a60a6e886105163f',
         'Discovery suffix list includes .cpp; HIP semantic options remain separately guarded'),
    'clang/test/Headers/lit.local.cfg':
        ('cdb4ada216d660956ceb7fd454db29213980da3e6032bce10244fe6a7effde58',
         'Adds %clang_src_sema_dir only; unmapped substitutions remain rejected'),
}

# This implementation test stub declares these names as its own field/parameter
# identifiers. It has no macros, conditional branches or compiler intrinsics.
# The original RTTI layout/IR obligations remain outside object acceptance.
TYPEINFO_STUB = 'clang/test/CodeGenCXX/typeinfo'
TYPEINFO_SHA = '22c2605cb1dfefff605ec8507e75227cb94a35cf9da86ab0bbb5c776e994db2a'
# GNU compatibility predefines need not start with an underscore. They cannot
# establish an always-active header guard even though the spelling is unreserved.
LEGACY_PLATFORM_MACROS = {'linux', 'unix', 'i386', 'i486', 'i586', 'i686',
                          'x86_64', 'amd64', 'arm', 'aarch64', 'mips', 'powerpc',
                          'sparc', 'sun', 'hpux', 'WIN32', 'WIN64'}


def configurations(source, root, excluded):
    result = []
    directory = source.parent
    while directory.is_relative_to(root / 'clang/test'):
        for spelling in ['lit.local.cfg', 'lit.local.cfg.py']:
            path = directory / spelling
            if not path.exists():
                continue
            name = str(path.relative_to(root))
            digest = hashlib.sha256(path.read_bytes()).hexdigest()
            policy = CONFIGS.get(name)
            if policy is None or digest != policy[0]:
                raise excluded('Unmapped or changed inherited native configuration: ' + name)
            result.append(dict(path=name, sha256=digest, policy=policy[1]))
        directory = directory.parent
    return result


def prepare(args, source, root, tracked, configs, source_policy, parts, excluded, cache=None):
    """Resolve literal -I%S paths and a fully frozen transitive quoted closure."""
    includes, stripped = [], []
    iterator = iter(args)
    for arg in iterator:
        if arg == '-I':
            arg = '-I' + next(iterator, '')
        if arg.startswith('-I'):
            value = arg[2:]
            if value != '%S' and not value.startswith('%S/'):
                raise excluded('Only literal corpus-local -I%S directories are mapped')
            path = (source.parent / value[3:].lstrip('/')).resolve()
            if not path.is_relative_to(root) or not path.is_dir():
                raise excluded('Include directory missing or outside pinned corpus: ' + value)
            name = str(path.relative_to(root))
            if name not in includes:
                includes.append(name)
        else:
            stripped.append(arg)
    cache_key = tuple(includes)
    if cache is not None and cache_key in cache:
        value = cache[cache_key]
        if isinstance(value, str):
            raise excluded(value)
        return stripped, *value
    files, texts, tokens, guards, definitions, edges = {}, [], set(), {}, set(), []
    active = set()
    used_directives = False

    def visit(path):
        nonlocal used_directives
        path = path.resolve()
        if not path.is_relative_to(root) or str(path.relative_to(root)) not in tracked:
            raise excluded('Input is not a tracked pinned corpus file: ' + str(path))
        name = str(path.relative_to(root))
        if name in active:
            raise excluded('Recursive include requires separate active-context evaluation: ' + name)
        if name in files:
            return
        active.add(name)
        data = path.read_bytes()
        text = re.sub(r'\\\r?\n', '', data.decode('utf-8', errors='strict').lstrip('\ufeff'))
        if re.search(r'\?\?[=/\'()!<>-]', text):
            raise excluded('Trigraph translation needs an explicit preprocessing-context review')
        # Strip comments but preserve literals for quoted include resolution.
        clean = parts.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0])
                          if m[0].startswith(('//', '/*')) else m[0], text)
        code = parts.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), clean)
        directives = [m for m in re.finditer(r'^[ \t]*(#|%:)[ \t]*(\w+)([^\n]*)', clean, re.M)
                      if code[m.start():m.end()].lstrip().startswith(('#', '%:'))]
        if directives:
            used_directives = True
        guard = None
        if directives and directives[0][2] == 'ifndef':
            first, last = directives[0], directives[-1]
            guard = first[3].strip()
            if (not re.fullmatch(r'[A-Za-z_]\w*', guard) or len(directives) < 3
                    or guard.startswith('_')
                    or guard in LEGACY_PLATFORM_MACROS
                    or clean[:first.start()].strip() or clean[last.end():].strip()
                    or last[2] != 'endif' or last[3].strip()
                    or directives[1][2] != 'define' or directives[1][3].strip() != guard):
                raise excluded('Unsupported header guard shape: ' + name)
            if guard in guards and guards[guard] != name:
                raise excluded('Header guard shared by different inputs: ' + guard)
            guards[guard] = name
        masked = list(text)
        for index, match in enumerate(directives):
            kind, tail = match[2], match[3].strip()
            if guard and index in {0, 1, len(directives) - 1}:
                pass
            elif kind == 'include':
                literal = re.fullmatch(r'(?:"([^"\n]+)"|<([^<>\n]+)>)', tail)
                if not literal:
                    raise excluded('Only literal corpus-local includes are mapped: ' + name)
                spelling = literal[1] or literal[2]
                candidates = ([path.parent / spelling] if literal[1] else []) + [
                    root / directory / spelling for directory in includes]
                found = next((p for p in candidates if p.is_file()), None)
                if found is None:
                    raise excluded('Literal include is missing from fixed corpus closure: ' + spelling)
                visit(found)
                edges.append(dict(source=name, spelling=spelling,
                                  kind='quoted' if literal[1] else 'angled',
                                  resolved=str(found.resolve().relative_to(root))))
            elif kind == 'pragma' and tail == 'once':
                pass
            elif kind == 'define':
                definition = re.match(r'([A-Za-z_]\w*)\b', tail)
                if not definition or '#' in tail or '%:' in tail:
                    raise excluded('Macro synthesis needs separate context review: ' + name)
                definitions.add(definition[1])
                tokens.update(source_policy(tail, allow_diagnostics=True))
            else:
                raise excluded('Conditional/directive needs active-context evaluation: ' + kind + ' in ' + name)
            masked[match.start():match.end()] = ' ' * (match.end() - match.start())
        body = ''.join(masked)
        policy_body = body
        if name == TYPEINFO_STUB:
            if hashlib.sha256(data).hexdigest() != TYPEINFO_SHA:
                raise excluded('Changed implementation type_info test stub')
            policy_body = re.sub(r'\b__(?:name|arg)\b', 'ordinary_identifier', body)
        tokens.update(source_policy(policy_body, allow_diagnostics=True))
        texts.append(body)
        files[name] = dict(path=name, sha256=hashlib.sha256(data).hexdigest())
        active.remove(name)

    try:
        visit(source)
        if definitions.intersection(guards):
            raise excluded('Source macro can disable a guarded input; active-context evaluation required')
    except excluded as error:
        if cache is not None:
            cache[cache_key] = str(error)
        raise
    extra = {}
    if used_directives or configs or includes:
        extra = dict(input_files=[files[n] for n in sorted(files)] + configs,
                     include_directories=includes,
                     include_edges=edges,
                     input_policy='Tracked transitive literal includes (angle form only via explicit -I); '
                                  'simple definitions/pragma once; unique outer header guards only; '
                                  'source-pinned type_info implementation stub identifiers if present; '
                                  'no arbitrary conditional branches or system-header fallback',
                     _oracle_text='\n'.join(texts))
    if cache is not None:
        cache[cache_key] = (tokens, extra)
    return stripped, tokens, extra

#!/usr/bin/env python3
"""Route Boost.Build C++ compilations to cppgm and record every invocation."""
import fcntl
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys
import time


def expand(arguments, depth=0):
    if depth > 8:
        raise ValueError('response files nested too deeply')
    result = []
    for argument in arguments:
        if argument.startswith('@'):
            result.extend(expand(shlex.split(Path(argument[1:]).read_text()), depth + 1))
        else:
            result.append(argument)
    return result


def route(arguments):
    language = None
    for index, argument in enumerate(arguments):
        if argument == '-x':
            language = arguments[index + 1]
        elif argument.startswith('-x'):
            language = argument[2:]
    sources = [a for a in arguments if not a.startswith('-')]
    if language in ('c', 'c-header', 'assembler', 'assembler-with-cpp') or (
        language is None and any(Path(a).suffix in ('.c', '.s', '.S') for a in sources)
    ):
        return 'host-c', os.environ['BOOST_CI_HOST_CC']
    if '-c' in arguments or '-E' in arguments or language in ('c++', 'c++-header') or any(
        Path(a).suffix in ('.cpp', '.cc', '.cxx', '.C') for a in sources
    ):
        return 'cppgm', os.environ['BOOST_CI_COMPILER']
    return 'host-link-or-query', os.environ['BOOST_CI_HOST_CXX']


def main():
    arguments = sys.argv[1:]
    expanded = expand(arguments)
    kind, compiler = route(expanded)
    start = time.monotonic()
    forwarded = expanded if kind == 'cppgm' else arguments
    process = subprocess.run([compiler, *forwarded], check=False)
    record = dict(cwd=os.getcwd(), route=kind, compiler=compiler, args=expanded,
                  returncode=process.returncode, seconds=time.monotonic() - start)
    with Path(os.environ['BOOST_CI_INVOCATIONS']).open('a') as stream:
        fcntl.flock(stream, fcntl.LOCK_EX)
        stream.write(json.dumps(record) + '\n')
    return process.returncode if process.returncode >= 0 else 128 - process.returncode


if __name__ == '__main__':
    sys.exit(main())

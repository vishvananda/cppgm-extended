#!/usr/bin/env python3
"""Time the native process selected by Boost.Build, preserving its output/status."""
import fcntl
import json
import os
from pathlib import Path
import resource
import subprocess
import sys
import time


def main():
    command = sys.argv[1:]
    before = resource.getrusage(resource.RUSAGE_CHILDREN)
    started = time.monotonic()
    result = subprocess.run(command, check=False, env=dict(os.environ, BOOST_CI_TIMER_ACTIVE='1'))
    elapsed = time.monotonic() - started
    after = resource.getrusage(resource.RUSAGE_CHILDREN)
    row = dict(command=command, cwd=os.getcwd(), returncode=result.returncode,
               cpu_seconds=after.ru_utime + after.ru_stime - before.ru_utime - before.ru_stime,
               elapsed_seconds=elapsed)
    with Path(os.environ['BOOST_CI_RUNTIME_INVOCATIONS']).open('a') as stream:
        fcntl.flock(stream, fcntl.LOCK_EX)
        stream.write(json.dumps(row) + '\n')
    return result.returncode if result.returncode >= 0 else 128 - result.returncode


if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
"""Exec a positive control with CPU/file limits and Linux address-space limits."""

import json
import os
from pathlib import Path
import resource
import signal
import sys


def main():
    if len(sys.argv) < 4 or sys.argv[2] != "--":
        raise ValueError("usage: run_resource_limited.py LIMITS -- COMMAND [ARG ...]")
    limits = json.loads(Path(sys.argv[1]).read_text())
    resources = {
        "cpu_seconds": resource.RLIMIT_CPU,
        "address_space_bytes": resource.RLIMIT_AS,
        "file_bytes": resource.RLIMIT_FSIZE,
    }
    if not isinstance(limits, dict) or set(limits) != set(resources):
        raise ValueError("limits must state cpu_seconds, address_space_bytes and file_bytes")
    for name, number in resources.items():
        value = limits[name]
        if type(value) is not int or value <= 0:
            raise ValueError(f"{name} must be a positive integer")
        # Darwin rejects ceilings below its existing shared-library mappings.
        # The owning Perl runner records this memory-only platform skip; the
        # correctness run, CPU limit and output-file limit still apply.
        if name == "address_space_bytes" and sys.platform == "darwin":
            continue
        resource.setrlimit(number, (value, value + 1 if name == "cpu_seconds" else value))
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    # Python ignores SIGPIPE and SIGXFSZ. Restore the dispositions a frontend
    # normally inherits from the Perl runner before replacing this process.
    for number in (signal.SIGPIPE, signal.SIGXFSZ, signal.SIGXCPU):
        signal.signal(number, signal.SIG_DFL)
    os.execvp(sys.argv[3], sys.argv[3:])


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, OverflowError) as error:
        print(f"resource limit setup failed: {error}", file=sys.stderr)
        sys.exit(70)

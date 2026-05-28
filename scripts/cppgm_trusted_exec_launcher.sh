#!/bin/bash
set -u

if [ "$#" -lt 1 ]; then
    echo "usage: $0 <executable> [args...]" >&2
    exit 127
fi

target=$1
shift

carrier=${CPPGM_TRUSTED_EXEC_CARRIER:-/Users/vishvananda/cppgm-safe/dev/cppgm++}
lockdir=${CPPGM_TRUSTED_EXEC_LOCK:-/tmp/cppgm-trusted-exec.lock}

if [ ! -x "$target" ]; then
    echo "cppgm trusted launcher: target is not executable: $target" >&2
    exit 126
fi

if [ ! -x "$carrier" ]; then
    echo "cppgm trusted launcher: carrier is not executable: $carrier" >&2
    exit 126
fi

while ! mkdir "$lockdir" 2>/dev/null; do
    sleep 0.1
done

backup="/tmp/cppgm-trusted-exec-carrier-backup.$$"

cleanup() {
    rc=$?
    if [ -f "$backup" ]; then
        cp "$backup" "$carrier" 2>/dev/null || true
        chmod +x "$carrier" 2>/dev/null || true
    fi
    rm -f "$backup"
    rmdir "$lockdir" 2>/dev/null || true
    exit "$rc"
}

trap cleanup EXIT INT TERM

cp "$carrier" "$backup" || exit 126
cp "$target" "$carrier" || exit 126
chmod +x "$carrier" || exit 126

"$carrier" "$@"
status=$?
exit "$status"

#!/bin/bash

set -e

if [ -f "${2%.t}.env" ]; then
  set -a
  . "${2%.t}.env"
  set +a
fi

compile_timeout="${CPPGM_BUILD_TEST_TIMEOUT_SEC:-45}"

app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
  read -r -a app_args <<< "$CPPGM_APP_ARGS"
fi

obj="$3.o"
rm -f "$obj"

flags=()
if [ -f "${2%.t}.compile.flags" ]; then
  read -r -a flags < "${2%.t}.compile.flags"
fi

: > "$3"
if command -v timeout >/dev/null 2>&1; then
  timeout "$compile_timeout" ./"$1" "${app_args[@]}" "${flags[@]}" -c -o "$obj" "$2" &> "$3.stdout"
else
  perl -e '
    use strict;
    use warnings;

    my $timeout = shift @ARGV;
    my $pid = fork();
    die "fork failed: $!" unless defined($pid);
    if ($pid == 0) {
      exec @ARGV;
      die "exec failed: $!";
    }

    my $timed_out = 0;
    local $SIG{ALRM} = sub {
      $timed_out = 1;
      kill q(TERM), $pid;
    };
    alarm($timeout);
    waitpid($pid, 0);
    alarm(0);

    if ($timed_out) {
      waitpid($pid, 0);
      exit 124;
    }

    exit(($? & 127) == 0 ? ($? >> 8) : (128 + ($? & 127)));
  ' "$compile_timeout" ./"$1" "${app_args[@]}" "${flags[@]}" -c -o "$obj" "$2" &> "$3.stdout"
fi
rm -f "$obj"

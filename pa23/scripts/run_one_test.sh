#!/bin/bash

app=$1
suffix=$2
testbase=$3
build_timeout=${CPPGM_BUILD_TEST_TIMEOUT_SEC:-30}
program_timeout=${CPPGM_PROGRAM_TEST_TIMEOUT_SEC:-10}

run_with_timeout() {
	if command -v timeout >/dev/null 2>&1
	then
		timeout "$@"
	elif command -v gtimeout >/dev/null 2>&1
	then
		gtimeout "$@"
	else
		local seconds=$1
		shift
		perl -e '
			use strict;
			use warnings;
			my ($seconds, @cmd) = @ARGV;
			my $pid = fork();
			die "fork failed: $!" unless defined $pid;
			if($pid == 0) {
				exec @cmd or die "exec failed: $!";
			}
			my $timed_out = 0;
			local $SIG{ALRM} = sub {
				$timed_out = 1;
				kill "TERM", $pid;
			};
			alarm $seconds;
			waitpid($pid, 0);
			alarm 0;
			if($timed_out) {
				kill "KILL", $pid;
				waitpid($pid, 0);
				exit 124;
			}
			if($? == -1) {
				exit 127;
			}
			if($? & 127) {
				exit 128 + ($? & 127);
			}
			exit ($? >> 8);
		' "$seconds" "$@"
	fi
}

rm -f "$testbase.$suffix.mir" \
	"$testbase.$suffix.program" \
	"$testbase.$suffix.program.exit_status" \
	"$testbase.$suffix.program.stdout" \
	"$testbase.$suffix.program.stderr" \
	"$testbase.$suffix.impl.stdout" \
	"$testbase.$suffix.impl.stderr" \
	"$testbase.$suffix.impl.exit_status"

cmd=(./"$app")
if [ -n "$LOWIR_NATIVE_TARGET" ]
then
	cmd+=(--target "$LOWIR_NATIVE_TARGET")
fi
cmd+=(--dump-machine-ir "$testbase.$suffix.mir")
cmd+=(-o "$testbase.$suffix.program")
cmd+=("$testbase.t")

run_with_timeout "$build_timeout" "${cmd[@]}" 1> $testbase.$suffix.impl.stdout 2> $testbase.$suffix.impl.stderr
impl_exit_status=$?

echo $impl_exit_status > $testbase.$suffix.impl.exit_status

if [ $impl_exit_status -eq 0 ]
then
	if [ -f "$testbase.stdin" ]
	then
		run_with_timeout "$program_timeout" ./$testbase.$suffix.program < $testbase.stdin 1> $testbase.$suffix.program.stdout 2> $testbase.$suffix.program.stderr
	else
		run_with_timeout "$program_timeout" ./$testbase.$suffix.program 1> $testbase.$suffix.program.stdout 2> $testbase.$suffix.program.stderr
	fi
	echo $? > $testbase.$suffix.program.exit_status
else
	rm -f "$testbase.$suffix.mir" \
		"$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr"
fi

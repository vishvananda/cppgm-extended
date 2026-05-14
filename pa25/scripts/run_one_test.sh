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

exec_path() {
	local path=$1
	if [[ "$path" == */* ]]; then
		printf '%s\n' "$path"
	else
		printf './%s\n' "$path"
	fi
}

appcmd=$(exec_path "$app")

rm -f "$testbase.$suffix.map" \
	"$testbase.$suffix.program" \
	"$testbase.$suffix.program.exit_status" \
	"$testbase.$suffix.program.stdout" \
	"$testbase.$suffix.program.stderr" \
	"$testbase.$suffix.impl.stdout" \
	"$testbase.$suffix.impl.stderr" \
	"$testbase.$suffix.impl.exit_status" \
	"$testbase.$suffix".*.obj

shopt -s nullglob
srcfiles=( "$testbase.t".* )
shopt -u nullglob

if [ ${#srcfiles[@]} -eq 0 ]; then
	echo "ERROR: no source files for $testbase" > "$testbase.$suffix.impl.stderr"
	echo 1 > "$testbase.$suffix.impl.exit_status"
	rm -f "$testbase.$suffix.impl.stdout" "$testbase.$suffix.impl.stderr"
	exit 0
fi

objfiles=()
compile_status=0
for src in "${srcfiles[@]}"
do
	index=${src##*.}
	objfile="$testbase.$suffix.$index.obj"
	objfiles+=( "$objfile" )
	run_with_timeout "$build_timeout" "$appcmd" -c -o "$objfile" "$src" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
	compile_status=$?
	if [ $compile_status -ne 0 ]; then
		break
	fi
done

if [ $compile_status -ne 0 ]; then
	echo $compile_status > "$testbase.$suffix.impl.exit_status"
	rm -f "${objfiles[@]}" \
		"$testbase.$suffix.map" \
		"$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr" \
		"$testbase.$suffix.impl.stdout" \
		"$testbase.$suffix.impl.stderr"
	exit 0
fi

cmd=( "$appcmd" --dump-link-map "$testbase.$suffix.map" -o "$testbase.$suffix.program" )
cmd+=( "${objfiles[@]}" )

run_with_timeout "$build_timeout" "${cmd[@]}" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
impl_exit_status=$?
echo $impl_exit_status > "$testbase.$suffix.impl.exit_status"

rm -f "${objfiles[@]}"

if [ $impl_exit_status -eq 0 ]
then
	program_path=$(exec_path "$testbase.$suffix.program")
	if [ -f "$testbase.stdin" ]
	then
		run_with_timeout "$program_timeout" "$program_path" < "$testbase.stdin" 1> "$testbase.$suffix.program.stdout" 2> "$testbase.$suffix.program.stderr"
	else
		run_with_timeout "$program_timeout" "$program_path" 1> "$testbase.$suffix.program.stdout" 2> "$testbase.$suffix.program.stderr"
	fi
	echo $? > "$testbase.$suffix.program.exit_status"
	rm -f "$testbase.$suffix.program" \
		"$testbase.$suffix.program.stderr"
else
	rm -f "$testbase.$suffix.map" \
		"$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr"
fi

rm -f "$testbase.$suffix.impl.stdout" "$testbase.$suffix.impl.stderr"

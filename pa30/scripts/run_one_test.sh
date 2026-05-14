#!/bin/bash

app=$1
suffix=$2
testbase=$3
build_timeout=${CPPGM_BUILD_TEST_TIMEOUT_SEC:-30}
program_timeout=${CPPGM_PROGRAM_TEST_TIMEOUT_SEC:-10}
app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
	read -r -a app_args <<< "$CPPGM_APP_ARGS"
fi

exec_path() {
	local path=$1
	if [[ "$path" == */* ]]; then
		printf '%s\n' "$path"
	else
		printf './%s\n' "$path"
	fi
}

appcmd=$(exec_path "$app")

command_exists() {
	local cmd=$1
	if [[ -z "$cmd" ]]; then
		return 1
	fi
	if [[ "$cmd" == */* ]]; then
		[[ -x "$cmd" ]]
		return $?
	fi
	command -v "$cmd" >/dev/null 2>&1
}

resolve_host_compiler() {
	local explicit=$1
	shift
	if [[ -n "$explicit" ]]; then
		local exe=${explicit%% *}
		if command_exists "$exe"; then
			printf '%s\n' "$explicit"
			return 0
		fi
	fi
	local candidate
	for candidate in "$@"; do
		if command_exists "$candidate"; then
			printf '%s\n' "$candidate"
			return 0
		fi
	done
	return 1
}

host_target_name() {
	case "$(uname -s)" in
		Darwin) printf '%s\n' macos ;;
		Linux) printf '%s\n' linux ;;
		*) printf '%s\n' "" ;;
	esac
}

host_target_triple_name() {
	case "$(uname -s)" in
		Darwin) printf '%s\n' x86_64-apple-darwin ;;
		Linux) printf '%s\n' x86_64-unknown-linux-gnu ;;
		*) printf '%s\n' "" ;;
	esac
}

host_cc=$(resolve_host_compiler "${CPPGM_HOST_CC:-${CC:-}}" clang-22 clang gcc cc c++) || {
	echo "Unable to resolve host C compiler" >&2
	exit 1
}
host_cxx=$(resolve_host_compiler "${CPPGM_HOST_CXX:-${CXX:-}}" clang++-22 clang++ g++ c++) || {
	echo "Unable to resolve host C++ compiler" >&2
	exit 1
}

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

rm -f "$testbase.$suffix.program" \
	"$testbase.$suffix.direct.program" \
	"$testbase.$suffix.mixed.program" \
	"$testbase.$suffix.program.exit_status" \
	"$testbase.$suffix.direct.program.exit_status" \
	"$testbase.$suffix.mixed.program.exit_status" \
	"$testbase.$suffix.program.stdout" \
	"$testbase.$suffix.direct.program.stdout" \
	"$testbase.$suffix.mixed.program.stdout" \
	"$testbase.$suffix.program.stderr" \
	"$testbase.$suffix.direct.program.stderr" \
	"$testbase.$suffix.mixed.program.stderr" \
	"$testbase.$suffix.impl.stdout" \
	"$testbase.$suffix.impl.stderr" \
	"$testbase.$suffix.impl.exit_status" \
	"$testbase.$suffix".*.obj
rm -rf "$testbase.$suffix.libdir"

shopt -s nullglob
srcfiles=( "$testbase.t".* )
shopt -u nullglob

if [ ${#srcfiles[@]} -eq 0 ]; then
	echo "ERROR: no source files for $testbase" > "$testbase.$suffix.impl.stderr"
	echo 1 > "$testbase.$suffix.impl.exit_status"
	rm -f "$testbase.$suffix.impl.stdout" "$testbase.$suffix.impl.stderr"
	exit 0
fi

flags=()
if [ -f "$testbase.flags" ]; then
	read -r -a flags < "$testbase.flags"
fi

libdir=""
helper_status=0
helper_sources=()
shopt -s nullglob
for helper in "$testbase.lib".*
do
	case "$helper" in
		*.c|*.cc|*.cpp|*.cxx) helper_sources+=( "$helper" ) ;;
	esac
done
shopt -u nullglob

if [ ${#helper_sources[@]} -ne 0 ]; then
	libdir="$testbase.$suffix.libdir"
	mkdir -p "$libdir"
	for src in "${helper_sources[@]}"
	do
		name=${src#"$testbase.lib."}
		name=${name%.*}
		obj="$libdir/lib$name.o"
		case "$src" in
			*.c) read -r -a compiler_cmd <<< "$host_cc" ;;
			*) read -r -a compiler_cmd <<< "$host_cxx" ;;
		esac
		run_with_timeout "$build_timeout" "${compiler_cmd[@]}" -c -o "$obj" "$src" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
		helper_status=$?
		if [ $helper_status -ne 0 ]; then
			break
		fi
	done
fi

resolved_flags=()
host_target=$(host_target_name)
host_target_triple=$(host_target_triple_name)
for flag in "${flags[@]}"
do
	if [ -n "$libdir" ]; then
		flag=${flag//__LIBDIR__/$libdir}
	fi
	if [ -n "$host_target" ]; then
		flag=${flag//__HOST_TARGET__/$host_target}
	fi
	if [ -n "$host_target_triple" ]; then
		flag=${flag//__HOST_TARGET_TRIPLE__/$host_target_triple}
	fi
	resolved_flags+=( "$flag" )
done
flags=( "${resolved_flags[@]}" )

objfiles=()
compile_status=$helper_status
for src in "${srcfiles[@]}"
do
	if [ $compile_status -ne 0 ]; then
		break
	fi
	index=${src##*.}
	objfile="$testbase.$suffix.$index.obj"
	objfiles+=( "$objfile" )
	run_with_timeout "$build_timeout" "$appcmd" "${app_args[@]}" "${flags[@]}" -c -o "$objfile" "$src" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
	compile_status=$?
	if [ $compile_status -ne 0 ]; then
		break
	fi
done

if [ $compile_status -ne 0 ]; then
	echo $compile_status > "$testbase.$suffix.impl.exit_status"
	rm -f "${objfiles[@]}"
else
	cmd=( "$appcmd" "${app_args[@]}" "${flags[@]}" -o "$testbase.$suffix.program" )
	cmd+=( "${objfiles[@]}" )

	run_with_timeout "$build_timeout" "${cmd[@]}" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
	compile_status=$?
	echo $compile_status > "$testbase.$suffix.impl.exit_status"
fi

direct_cmd=( "$appcmd" "${app_args[@]}" "${flags[@]}" -o "$testbase.$suffix.direct.program" )
direct_cmd+=( "${srcfiles[@]}" )
run_with_timeout "$build_timeout" "${direct_cmd[@]}" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
direct_status=$?

mixed_status=$compile_status
if [ ${#srcfiles[@]} -gt 1 ]; then
	mixed_cmd=( "$appcmd" "${app_args[@]}" "${flags[@]}" -o "$testbase.$suffix.mixed.program" "${objfiles[0]}" )
	for ((i = 1; i < ${#srcfiles[@]}; ++i))
	do
		mixed_cmd+=( "${srcfiles[$i]}" )
	done
	run_with_timeout "$build_timeout" "${mixed_cmd[@]}" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
	mixed_status=$?
fi

run_program() {
	local program=$1
	local stdout_file=$2
	local stderr_file=$3
	local status_file=$4
	local program_path
	program_path=$(exec_path "$program")
	if [ -f "$testbase.stdin" ]
	then
		run_with_timeout "$program_timeout" "$program_path" < "$testbase.stdin" 1> "$stdout_file" 2> "$stderr_file"
	else
		run_with_timeout "$program_timeout" "$program_path" 1> "$stdout_file" 2> "$stderr_file"
	fi
	echo $? > "$status_file"
}

if [ $direct_status -eq 0 ]
then
	run_program "$testbase.$suffix.direct.program" \
		"$testbase.$suffix.direct.program.stdout" \
		"$testbase.$suffix.direct.program.stderr" \
		"$testbase.$suffix.direct.program.exit_status"
fi

if [ ${#srcfiles[@]} -gt 1 ] && [ $mixed_status -eq 0 ]
then
	run_program "$testbase.$suffix.mixed.program" \
		"$testbase.$suffix.mixed.program.stdout" \
		"$testbase.$suffix.mixed.program.stderr" \
		"$testbase.$suffix.mixed.program.exit_status"
fi

if [ $compile_status -eq 0 ]
then
	run_program "$testbase.$suffix.program" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr" \
		"$testbase.$suffix.program.exit_status"
fi

if [ $compile_status -ne $direct_status ]
then
	echo "ERROR: direct source link status differs from separate compile/link" >> "$testbase.$suffix.impl.stderr"
	echo 1 > "$testbase.$suffix.impl.exit_status"
	rm -f "$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr"
elif [ ${#srcfiles[@]} -gt 1 ] && [ $compile_status -ne $mixed_status ]
then
	echo "ERROR: mixed source/object link status differs from separate compile/link" >> "$testbase.$suffix.impl.stderr"
	echo 1 > "$testbase.$suffix.impl.exit_status"
	rm -f "$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr"
elif [ $compile_status -eq 0 ]
then
	if ! cmp -s "$testbase.$suffix.program.stdout" "$testbase.$suffix.direct.program.stdout" || \
	   ! cmp -s "$testbase.$suffix.program.exit_status" "$testbase.$suffix.direct.program.exit_status"
	then
		echo "ERROR: direct source link output differs from separate compile/link" >> "$testbase.$suffix.impl.stderr"
		echo 1 > "$testbase.$suffix.impl.exit_status"
		rm -f "$testbase.$suffix.program" \
			"$testbase.$suffix.program.exit_status" \
			"$testbase.$suffix.program.stdout" \
			"$testbase.$suffix.program.stderr"
	elif [ ${#srcfiles[@]} -gt 1 ] && \
	     ( ! cmp -s "$testbase.$suffix.program.stdout" "$testbase.$suffix.mixed.program.stdout" || \
	       ! cmp -s "$testbase.$suffix.program.exit_status" "$testbase.$suffix.mixed.program.exit_status" )
	then
		echo "ERROR: mixed source/object link output differs from separate compile/link" >> "$testbase.$suffix.impl.stderr"
		echo 1 > "$testbase.$suffix.impl.exit_status"
		rm -f "$testbase.$suffix.program" \
			"$testbase.$suffix.program.exit_status" \
			"$testbase.$suffix.program.stdout" \
			"$testbase.$suffix.program.stderr"
	fi
fi

rm -f "$testbase.$suffix.impl.stdout" "$testbase.$suffix.impl.stderr" \
	"$testbase.$suffix.direct.program" \
	"$testbase.$suffix.direct.program.exit_status" \
	"$testbase.$suffix.direct.program.stdout" \
	"$testbase.$suffix.direct.program.stderr" \
	"$testbase.$suffix.mixed.program" \
	"$testbase.$suffix.mixed.program.exit_status" \
	"$testbase.$suffix.mixed.program.stdout" \
	"$testbase.$suffix.mixed.program.stderr"
rm -rf "$testbase.$suffix.libdir"

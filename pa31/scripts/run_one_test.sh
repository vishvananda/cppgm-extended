#!/bin/bash

app=$1
suffix=$2
testbase=$3
repo_root=$(cd "$(dirname "$0")/../.." && pwd)
build_timeout=${CPPGM_BUILD_TEST_TIMEOUT_SEC:-30}
program_timeout=${CPPGM_PROGRAM_TEST_TIMEOUT_SEC:-10}
app_args=()
if [ -n "${CPPGM_APP_ARGS:-}" ]; then
	read -r -a app_args <<< "$CPPGM_APP_ARGS"
fi

if ! command -v rg >/dev/null 2>&1; then
	export PATH="$repo_root/scripts/test_shell_tools:$PATH"
fi

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

host_cxx="${CXX:-c++}"
host_cc="${CC:-cc}"
host_os=$(uname -s)
shared_ext=so
host_tag=$(printf '%s' "$host_os" | tr '[:upper:]' '[:lower:]')
if [ "$host_os" = "Darwin" ]; then
	shared_ext=dylib
	host_tag=macos
fi

rm -f "$testbase.$suffix.program" \
	"$testbase.$suffix.program.exit_status" \
	"$testbase.$suffix.program.stdout" \
	"$testbase.$suffix.program.stderr" \
	"$testbase.$suffix.inspect" \
	"$testbase.$suffix.impl.stdout" \
	"$testbase.$suffix.impl.stderr" \
	"$testbase.$suffix.impl.exit_status" \
	"$testbase.$suffix.impl.link_command" \
	"$testbase.$suffix.impl.link_verbose" \
	"$testbase.$suffix.impl.unresolved_symbols" \
	"$testbase.$suffix".*.o
rm -rf "$testbase.$suffix.libdir"

shopt -s nullglob
srcfiles=()
for candidate in "$testbase.t".*
do
	if [[ "$candidate" =~ \.t\.[0-9]+$ ]]; then
		srcfiles+=( "$candidate" )
	fi
done
shopt -u nullglob

if [ ${#srcfiles[@]} -eq 0 ]; then
	echo "ERROR: no source files for $testbase" > "$testbase.$suffix.impl.stderr"
	echo 1 > "$testbase.$suffix.impl.exit_status"
	rm -f "$testbase.$suffix.impl.stdout" "$testbase.$suffix.impl.stderr"
	exit 0
fi

compile_flags=()
if [ -f "$testbase.compile.flags" ]; then
	read -r -a compile_flags < "$testbase.compile.flags"
fi

link_flags=()
if [ -f "$testbase.link.flags" ]; then
	read -r -a link_flags < "$testbase.link.flags"
fi
use_cppgm_link=false
if [ -f "$testbase.cppgm-link" ]; then
	use_cppgm_link=true
fi

need_helper_object=false
need_helper_static=false
need_helper_shared=false
for flag in "${link_flags[@]}"
do
	case "$flag" in
		*.o) need_helper_object=true ;;
		*.a) need_helper_static=true ;;
		*.__SHARED_EXT__|*.dylib|*.so) need_helper_shared=true ;;
	esac
done
if [ "$need_helper_static" = true ] || [ "$need_helper_shared" = true ]; then
	need_helper_object=true
fi

run_args=()
if [ -f "$testbase.argv" ]; then
	read -r -a run_args < "$testbase.argv"
fi

libdir=""
helper_status=0
helper_provider_files=()
helper_referencer_files=()
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
		staticlib="$libdir/lib$name.a"
		sharedlib="$libdir/lib$name.$shared_ext"
		case "$src" in
			*.c) compiler="$host_cc" ;;
			*) compiler="$host_cxx" ;;
		esac
		if [ "$need_helper_object" = true ]; then
			run_with_timeout "$build_timeout" "$compiler" -fPIC -c -o "$obj" "$src" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
			helper_status=$?
			if [ $helper_status -ne 0 ]; then
				break
			fi
			helper_provider_files+=( "$obj" )
			helper_referencer_files+=( "$obj" )
		fi
		if [ "$need_helper_static" = true ]; then
			run_with_timeout "$build_timeout" ar rcs "$staticlib" "$obj" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
			helper_status=$?
			if [ $helper_status -ne 0 ]; then
				break
			fi
			helper_provider_files+=( "$staticlib" )
		fi
		if [ "$need_helper_shared" = true ]; then
			if [ "$host_os" = "Darwin" ]; then
				run_with_timeout "$build_timeout" "$host_cc" -dynamiclib -o "$sharedlib" "$obj" \
					-Wl,-install_name,@rpath/$(basename "$sharedlib") \
					>> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
			else
				run_with_timeout "$build_timeout" "$host_cc" -shared -o "$sharedlib" "$obj" \
					>> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
			fi
			helper_status=$?
			if [ $helper_status -ne 0 ]; then
				break
			fi
			helper_provider_files+=( "$sharedlib" )
		fi
	done
fi

resolved_link_flags=()
for flag in "${link_flags[@]}"
do
	if [ -n "$libdir" ]; then
		resolved_link_flags+=( "${flag//__LIBDIR__/$libdir}" )
		resolved_link_flags[$((${#resolved_link_flags[@]} - 1))]=${resolved_link_flags[$((${#resolved_link_flags[@]} - 1))]//__SHARED_EXT__/$shared_ext}
	else
		resolved_link_flags+=( "${flag//__SHARED_EXT__/$shared_ext}" )
	fi
done
link_flags=( "${resolved_link_flags[@]}" )

inspect_cmd_file=""
if [ -f "$testbase.inspect.cmd.$host_tag" ]; then
	inspect_cmd_file="$testbase.inspect.cmd.$host_tag"
elif [ -f "$testbase.inspect.cmd" ]; then
	inspect_cmd_file="$testbase.inspect.cmd"
fi
inspect_expect_file=""
if [ -f "$testbase.inspect.expect.$host_tag" ]; then
	inspect_expect_file="$testbase.inspect.expect.$host_tag"
elif [ -f "$testbase.inspect.expect" ]; then
	inspect_expect_file="$testbase.inspect.expect"
fi
inspect_plan_file=""
if [ -f "$testbase.inspect.plan.$host_tag" ]; then
	inspect_plan_file="$testbase.inspect.plan.$host_tag"
elif [ -f "$testbase.inspect.plan" ]; then
	inspect_plan_file="$testbase.inspect.plan"
fi

objfiles=()
compile_status=$helper_status
for src in "${srcfiles[@]}"
do
	if [ $compile_status -ne 0 ]; then
		break
	fi
	index=${src##*.}
	objfile="$testbase.$suffix.$index.o"
	objfiles+=( "$objfile" )
	run_with_timeout "$build_timeout" "$appcmd" "${app_args[@]}" "${compile_flags[@]}" -c -o "$objfile" "$src" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
	compile_status=$?
	if [ $compile_status -ne 0 ]; then
		break
	fi
done

if [ $compile_status -ne 0 ]; then
	echo $compile_status > "$testbase.$suffix.impl.exit_status"
	rm -f "${objfiles[@]}"
	exit 0
fi

if [ "$use_cppgm_link" = true ]; then
	link_cmd=( "$appcmd" "${app_args[@]}" -o "$testbase.$suffix.program" )
else
	link_cmd=( "$host_cxx" -o "$testbase.$suffix.program" )
fi
link_cmd+=( "${objfiles[@]}" )
if [ -n "$libdir" ]; then
	link_cmd+=( "-Wl,-rpath,$libdir" )
fi
link_cmd+=( "${link_flags[@]}" )
printf '%q ' "${link_cmd[@]}" > "$testbase.$suffix.impl.link_command"
printf '\n' >> "$testbase.$suffix.impl.link_command"

run_with_timeout "$build_timeout" "${link_cmd[@]}" >> "$testbase.$suffix.impl.stdout" 2>> "$testbase.$suffix.impl.stderr"
impl_exit_status=$?
echo $impl_exit_status > "$testbase.$suffix.impl.exit_status"

if [ $impl_exit_status -ne 0 ]; then
	if [ "$use_cppgm_link" = false ]; then
		verbose_link_cmd=( "$host_cxx" -v -o "$testbase.$suffix.program" )
		verbose_link_cmd+=( "${objfiles[@]}" )
		if [ -n "$libdir" ]; then
			verbose_link_cmd+=( "-Wl,-rpath,$libdir" )
		fi
		verbose_link_cmd+=( "${link_flags[@]}" )
		run_with_timeout "$build_timeout" "${verbose_link_cmd[@]}" > "$testbase.$suffix.impl.link_verbose" 2>&1 || true
	fi

	explicit_provider_flags=()
	for flag in "${link_flags[@]}"
	do
		case "$flag" in
			*.o|*.a|*.dylib|*.so)
				if [ -e "$flag" ]; then
					explicit_provider_flags+=( "$flag" )
				fi
				;;
		esac
	done
	nm_referencers=( "${objfiles[@]}" "${helper_referencer_files[@]}" )
	for file in "${explicit_provider_flags[@]}"
	do
		case "$file" in
			*.o) nm_referencers+=( "$file" ) ;;
		esac
	done
	nm_providers=( "${objfiles[@]}" "${helper_provider_files[@]}" "${explicit_provider_flags[@]}" )
	report_cmd=( perl "$repo_root/scripts/write_unresolved_symbol_report.pl" \
		--output "$testbase.$suffix.impl.unresolved_symbols" \
		--host-cxx "$host_cxx" )
	for file in "${nm_referencers[@]}"
	do
		report_cmd+=( --referencer "$file" )
	done
	for file in "${nm_providers[@]}"
	do
		report_cmd+=( --provider "$file" )
	done
	"${report_cmd[@]}" >/dev/null 2>&1 || true
fi

if [ $impl_exit_status -eq 0 ] && [ -n "$inspect_expect_file" ]; then
	perl "$repo_root/scripts/check_object_expectations.pl" \
		"$inspect_expect_file" "${objfiles[@]}" > "$testbase.$suffix.inspect" \
		2>> "$testbase.$suffix.impl.stderr"
	inspect_status=$?
	if [ $inspect_status -ne 0 ]; then
		impl_exit_status=$inspect_status
		echo $impl_exit_status > "$testbase.$suffix.impl.exit_status"
	fi
elif [ $impl_exit_status -eq 0 ] && [ -n "$inspect_plan_file" ] && { [ "$suffix" = "ref" ] || [ -f "$testbase.ref.inspect" ]; }; then
	perl "$repo_root/scripts/check_object_expectations.pl" --record \
		"$inspect_plan_file" "${objfiles[@]}" > "$testbase.$suffix.inspect" \
		2>> "$testbase.$suffix.impl.stderr"
	inspect_status=$?
	if [ $inspect_status -ne 0 ]; then
		impl_exit_status=$inspect_status
		echo $impl_exit_status > "$testbase.$suffix.impl.exit_status"
	fi
elif [ $impl_exit_status -eq 0 ] && [ -n "$inspect_cmd_file" ]; then
	inspect_cmd=$(cat "$inspect_cmd_file")
	for idx in "${!objfiles[@]}"
	do
		n=$((idx + 1))
		inspect_cmd=${inspect_cmd//__OBJ${n}__/${objfiles[$idx]}}
	done
	inspect_cmd=${inspect_cmd//__PROGRAM__/$testbase.$suffix.program}
	inspect_cmd=${inspect_cmd//__SHARED_EXT__/$shared_ext}
	if [ -n "$libdir" ]; then
		inspect_cmd=${inspect_cmd//__LIBDIR__/$libdir}
	fi
	bash -lc "$inspect_cmd" > "$testbase.$suffix.inspect" 2>> "$testbase.$suffix.impl.stderr"
	inspect_status=$?
	if [ $inspect_status -ne 0 ]; then
		impl_exit_status=$inspect_status
		echo $impl_exit_status > "$testbase.$suffix.impl.exit_status"
	fi
fi

if [ $impl_exit_status -eq 0 ]
then
	rm -f "${objfiles[@]}"
	program_path=$(exec_path "$testbase.$suffix.program")
	if [ -f "$testbase.stdin" ]
	then
			run_with_timeout "$program_timeout" "$program_path" "${run_args[@]}" < "$testbase.stdin" 1> "$testbase.$suffix.program.stdout" 2> "$testbase.$suffix.program.stderr"
	else
			run_with_timeout "$program_timeout" "$program_path" "${run_args[@]}" 1> "$testbase.$suffix.program.stdout" 2> "$testbase.$suffix.program.stderr"
	fi
	echo $? > "$testbase.$suffix.program.exit_status"
	rm -f "$testbase.$suffix.program" \
		"$testbase.$suffix.program.stderr"
else
	rm -f "$testbase.$suffix.program" \
		"$testbase.$suffix.program.exit_status" \
		"$testbase.$suffix.program.stdout" \
		"$testbase.$suffix.program.stderr"
fi

if [ $impl_exit_status -eq 0 ]; then
	rm -f "$testbase.$suffix.impl.stdout" \
		"$testbase.$suffix.impl.stderr" \
		"$testbase.$suffix.impl.link_command" \
		"$testbase.$suffix.impl.link_verbose" \
		"$testbase.$suffix.impl.unresolved_symbols"
	rm -rf "$libdir"
fi

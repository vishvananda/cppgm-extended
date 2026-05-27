set -eu
if [ "$(uname -s)" != "Darwin" ]; then
  echo compact_unwind_large_frame_fallback_ok 1
  exit 0
fi

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/cppgm-compact-unwind-overflow.XXXXXX")
trap 'rm -rf "$tmpdir"' EXIT

"${CPPGM_CPPEH_APP:-../dev/cppeh}" -c -o "$tmpdir/victim.o" \
  tests/general/200-host-compact-unwind-large-frame-fallback.lowir

objdump_tool=$(xcrun -f llvm-objdump 2>/dev/null || true)
if [ -z "$objdump_tool" ] && [ -x /usr/local/opt/llvm/bin/llvm-objdump ]; then
  objdump_tool=/usr/local/opt/llvm/bin/llvm-objdump
fi
if [ -z "$objdump_tool" ]; then
  echo missing_llvm_objdump
  exit 1
fi

unwind_info=$("$objdump_tool" --macho --unwind-info "$tmpdir/victim.o")
if printf '%s\n' "$unwind_info" | grep -q '_victim'; then
  echo compact_unwind_overflow_row_present
  printf '%s\n' "$unwind_info"
  exit 1
fi
otool -l "$tmpdir/victim.o" | grep -q 'sectname __eh_frame'

cat > "$tmpdir/driver.cpp" <<'CPP'
extern "C" long long victim();
extern "C" long long touch(void *)
{
  throw 7;
}

int main()
{
  try {
    (void)victim();
    return 1;
  } catch (int value) {
    return value == 7 ? 0 : 2;
  }
}
CPP

if [ -n "${CPPGM_HOST_CXX:-}" ]; then
  host_cxx=($CPPGM_HOST_CXX)
elif [ -n "${CXX:-}" ]; then
  host_cxx=($CXX)
else
  host_cxx=(c++)
fi
"${host_cxx[@]}" -std=gnu++11 -o "$tmpdir/run" "$tmpdir/driver.cpp" "$tmpdir/victim.o"
"$tmpdir/run"

echo compact_unwind_large_frame_fallback_ok 1

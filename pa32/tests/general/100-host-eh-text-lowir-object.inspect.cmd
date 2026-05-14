set -eu
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/cppgm-host-eh-lowir.XXXXXX")
trap 'rm -rf "$tmpdir"' EXIT
"${CPPGM_CPPLINK_APP:-../dev/cpplink}" -c -o "$tmpdir/cpplink.o" tests/general/100-host-eh-text-lowir-object.lowir
"${CPPGM_CPPEH_APP:-../dev/cppeh}" -c -o "$tmpdir/cppeh.o" tests/general/100-host-eh-text-lowir-object.lowir
nm -u "$tmpdir/cpplink.o" | grep -q 'gxx_personality'
nm -u "$tmpdir/cppeh.o" | grep -q 'gxx_personality'
echo text_lowir_host_eh_object_ok 1

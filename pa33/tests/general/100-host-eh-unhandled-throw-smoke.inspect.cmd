set -eu
nm -u "__OBJ1__" | grep -q 'cxa_allocate_exception'
nm -u "__OBJ1__" | grep -q 'cxa_throw'
if nm "__OBJ1__" | grep -q 'cppgm_eh_'; then
  echo unexpected_cppgm_eh_symbols
  exit 1
fi
echo host_eh_surface_ok 1

set -eu
perl ../scripts/compare_host_defined_symbols.pl \
  --obj "__OBJ1__" \
  --source "tests/general/200-host-namespaced-enum-template-arg-mangling.t.1" \
  --host-cxx "${CPPGM_HOST_CXX:-${CXX:-c++}}" \
  --demangled-contains 'semantic_overload::select_constructor_from_exprs('

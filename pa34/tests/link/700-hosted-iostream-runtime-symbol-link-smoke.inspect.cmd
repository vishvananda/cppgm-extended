set -eu
nm -u "__OBJ1__" > "__OBJ1__.undefined.raw"
nm -u "__OBJ1__" | awk '{print $NF}' | c++filt > "__OBJ1__.demangled"
grep -Eq 'std::.*cout' "__OBJ1__.demangled"
if ! perl scripts/compare_host_undefined_symbols.pl --obj "__OBJ1__" \
    --match 'std::.*endl.*basic_ostream<char, .*>&\)$' \
    --match 'std::.*operator<<.*char const\*\)$' \
    --optional-match 'std::.*basic_ostream<char, .*>::flush\(\)$' \
    --optional-match 'std::.*basic_ostream<char, .*>::put\(char\)$' \
    > "__OBJ1__.inspect.tmp" 2>&1; then
  if ! grep -Eq 'std::.*basic_ostream<char, .*>::put\(char\)$' "__OBJ1__.demangled" ||
     ! grep -Eq 'std::.*basic_ostream<char, .*>::flush\(\)$' "__OBJ1__.demangled"; then
    cat "__OBJ1__.inspect.tmp"
    exit 1
  fi
fi
rm -f "__OBJ1__.inspect.tmp"
if grep -Fq 'std::basic_streambuf<char, std::char_traits<char>>::pptr()' "__OBJ1__.demangled"; then
  echo unexpected_pptr_unresolved
  exit 1
fi
if grep -Fq 'std::basic_streambuf<char, std::char_traits<char>>::epptr()' "__OBJ1__.demangled"; then
  echo unexpected_epptr_unresolved
  exit 1
fi
if grep -Fq 'std::basic_streambuf<char, std::char_traits<char>>::__check_invariants() const' "__OBJ1__.demangled"; then
  echo unexpected_check_invariants_unresolved
  exit 1
fi
if nm -u "__OBJ1__" | grep -q '__cppgm_'; then
  echo unexpected_cppgm_runtime_symbol
  exit 1
fi
if nm -u "__OBJ1__" | grep -q 'cppgm_host_num_put_char_put'; then
  echo unexpected_host_num_put_bridge_symbol
  exit 1
fi
echo hosted_iostream_runtime_surface_ok 1

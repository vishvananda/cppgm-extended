set -eu
nm -u "__OBJ1__" > "__OBJ1__.undefined.raw"
nm -u "__OBJ1__" | awk '{print $NF}' | c++filt > "__OBJ1__.undefined.demangled"
nm "__OBJ1__" | c++filt > "__OBJ1__.all.demangled"

if ! perl scripts/compare_host_undefined_symbols.pl --obj "__OBJ1__" \
  --match 'std::.*basic_ofstream<char, .*>::basic_ofstream(\[abi:[^]]+\])?\(char const\*, .*\)' \
  --match 'std::.*basic_ofstream<char, .*>::~basic_ofstream\(\)' \
  --match 'std::.*operator<<.*char const\*\)$' \
  --optional-match 'std::.*basic_filebuf<char, .*>::basic_filebuf\(\)' \
  --optional-match 'std::.*basic_filebuf<char, .*>::open\(char const\*, .*\)' \
  --optional-match 'std::.*basic_filebuf<char, .*>::~basic_filebuf\(\)' \
  --optional-match 'std::.*basic_ios<char, .*>::~basic_ios\(\)' \
  --optional-match 'std::.*basic_ostream<char, .*>::~basic_ostream\(\)' \
  --optional-match 'VTT for std::.*basic_ofstream<char, .*>' \
  > "__OBJ1__.inspect.tmp" 2>&1; then
  local_ctor=no
  local_dtor=no
  host_ctor=no
  host_filebuf_ctor=no
  host_filebuf_open=no
  host_filebuf_dtor=no
  grep -Eq '^[0-9A-Fa-f]+ [A-Za-z] std::.*basic_ofstream<char, .*>::basic_ofstream(\[abi:[^]]+\])?\(char const\*, .*\)' "__OBJ1__.all.demangled" && local_ctor=yes
  grep -Eq '^[0-9A-Fa-f]+ [A-Za-z] std::.*basic_ofstream<char, .*>::~basic_ofstream\(\)' "__OBJ1__.all.demangled" && local_dtor=yes
  grep -Eq 'std::.*basic_ofstream<char, .*>::basic_ofstream(\[abi:[^]]+\])?\(char const\*, .*\)' "__OBJ1__.undefined.demangled" && host_ctor=yes
  grep -Eq 'std::.*basic_filebuf<char, .*>::basic_filebuf\(\)' "__OBJ1__.undefined.demangled" && host_filebuf_ctor=yes
  grep -Eq 'std::.*basic_filebuf<char, .*>::open\(char const\*, .*\)' "__OBJ1__.undefined.demangled" && host_filebuf_open=yes
  grep -Eq 'std::.*basic_filebuf<char, .*>::~basic_filebuf\(\)' "__OBJ1__.undefined.demangled" && host_filebuf_dtor=yes

  if { [ "$local_ctor" != yes ] || [ "$local_dtor" != yes ] ||
       [ "$host_filebuf_ctor" != yes ] || [ "$host_filebuf_open" != yes ] ||
       [ "$host_filebuf_dtor" != yes ]; } &&
     { [ "$host_ctor" != yes ] || [ "$local_dtor" != yes ] ||
       [ "$host_filebuf_dtor" != yes ]; }; then
    cat "__OBJ1__.inspect.tmp"
    exit 1
  fi
fi
rm -f "__OBJ1__.inspect.tmp"

if grep -Eq '^[0-9A-Fa-f]+ [A-Za-z] vtable for std::.*basic_ofstream<char, .*char_traits<char>.*>' "__OBJ1__.all.demangled"; then
  echo unexpected_local_basic_ofstream_vtable
  exit 1
fi

if grep -Eq '^[0-9A-Fa-f]+ [A-Za-z] typeinfo for std::.*basic_ofstream<char, .*char_traits<char>.*>' "__OBJ1__.all.demangled"; then
  echo unexpected_local_basic_ofstream_typeinfo
  exit 1
fi

echo hosted_ofstream_runtime_surface_ok 1

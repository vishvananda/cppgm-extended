set -eu
perl scripts/compare_host_undefined_symbols.pl --obj "__OBJ1__" \
  --match 'std::logic_error::logic_error\(std::.*basic_string<char, .*char_traits<char>, .*allocator<char>.*> const&\)' \
  --match 'std::logic_error::logic_error\(std::logic_error const&\)' \
  --match 'std::runtime_error::runtime_error\(std::.*basic_string<char, .*char_traits<char>, .*allocator<char>.*> const&\)'
echo hosted_standard_exception_string_ctor_surface_ok 1

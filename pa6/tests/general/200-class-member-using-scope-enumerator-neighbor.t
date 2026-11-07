// N3485 7.3.3: namespace-owned unscoped enumerator imports remain valid. expect exit 0
enum E{value=7};using E::value;static_assert(value==7,"");

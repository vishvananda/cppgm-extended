// CXX11-125: adopted P1787R6 lookup; alias substitution retains its required checks.
// expect reject
template<bool> struct B {};
template<bool Flag> using Alias = typename B<Flag>::type;
Alias<true> required;

// CXX11-125: adopted P1787R6 lookup; alias substitution retains its required checks.
// expect exit 0
template<bool> struct B { using type = int; };
template<bool Flag> using Alias = typename B<Flag>::type;
int main() { Alias<true> x=7; return x-7; }

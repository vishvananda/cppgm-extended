// CXX11-125: adopted P1787R6 lookup; alias substitution retains its required checks.
// expect exit 0
template<bool> struct B { using type = int; };
template<bool B> using Alias = typename ::B<B>::type;
int main() { Alias<false> x=9; return x-9; }

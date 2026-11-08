// expect exit 0
// I13: Ordinary unambiguous imported literal lookup and explicit operator calls stay valid.
namespace Other { int operator "" _which(unsigned long long n) { return n+1; } } using namespace Other;
int main() { return 10_which!=11 || Other::operator "" _which(20)!=21; }

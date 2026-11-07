// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// Instantiate the ordinary explicit call so this is not an unused IFNDR template oracle.
// expect reject
template<char... C> int operator "" _bad(unsigned long long) { return sizeof...(C); }
int main() { return operator "" _bad<'x'>(1ULL); }

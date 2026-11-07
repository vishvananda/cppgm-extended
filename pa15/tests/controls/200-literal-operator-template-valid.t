// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// expect exit 0
template<char... C> int operator "" _digits() { return sizeof...(C); }
int main() { return 123_digits != 3 || operator "" _digits<>() != 0; }

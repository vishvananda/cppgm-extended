// N3485 13.5.8p2-p6: validate literal operator declaration scope and parameter forms.
// Instantiate the ordinary explicit call so this is not an unused IFNDR template oracle.
// expect reject
template<class... T> int operator "" _bad() { return sizeof...(T); }
int main() { return operator "" _bad<int>(); }

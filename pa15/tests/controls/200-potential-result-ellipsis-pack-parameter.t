// N3485 3.2,8.3.6p9,CWG2082/2083: consume actual conversions; retain default-parameter prohibition.
// expect reject
void sink(...); template<int... N> void outer(int... values[N]) { struct Local { Local() { sink(values...); } }; } int main() { int array[4]; outer<4>(array); }

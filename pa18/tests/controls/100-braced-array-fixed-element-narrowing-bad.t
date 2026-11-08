// expect reject
// I13: Array bound deduction does not waive element narrowing after a fixed type is known.
template<int N> int count(const char (&)[N]) { return N; } int main() { return count({1,300,3}); }

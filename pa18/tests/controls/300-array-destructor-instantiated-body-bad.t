// expect reject
// I13: The same deleted-array-destructor obligation is a hard error in an instantiated function body.
struct Bad { ~Bad() = delete; }; template<class T> void make() { T{}; } int main() { make<Bad[2]>(); }

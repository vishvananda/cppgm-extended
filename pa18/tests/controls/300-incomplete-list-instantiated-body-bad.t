// expect reject
// Incomplete temporary in an actually instantiated body remains an error.
struct Incomplete; template<class T> void body() { T{}; }
int main() { body<Incomplete>(); }

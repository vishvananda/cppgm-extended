// expect reject
// Scalar new in an instantiated body is not substitution failure.
template<class T> void body() { new T(1, 2); }
int main() { body<int>(); }
